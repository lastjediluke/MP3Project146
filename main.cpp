#include <stdio.h>
#include "utilities.h"
#include "io.hpp"
#include "MP3SPI.hpp"
#include "MP3GPIO.hpp"
#include "FreeRTOS.h"
#include "tasks.hpp"
#include "task.h"
#include "uart0_min.h"
#include "LPC17xx.h"
#include "INTDrv.hpp"
#include "lpc_isr.h"
#include <string.h>
// #include "testForMP3.cpp"


INTDrv * IntDriver = INTDrv::instance();

SemaphoreHandle_t downSemaphore = NULL;
SemaphoreHandle_t selectSemaphore = NULL;
SemaphoreHandle_t backSemaphore = NULL;
SemaphoreHandle_t xSemaphore = NULL;

bool downButton = 0;
bool selectButton = 0;
bool backButton = 0;

char **statePtr;
char **songPlayingPtr;


MP3SPI instance;

MP3GPIO lcd_cs(1, 0);                 

MP3GPIO sw1 (9, 1);		
MP3GPIO sw2 (10, 1);		
MP3GPIO sw3 (14, 1);		
MP3GPIO sw4 (15, 1);

MP3GPIO led1 (0, 1);
MP3GPIO led2 (1, 1);
MP3GPIO led3 (4, 1);
MP3GPIO led4 (8, 1);

// ### Now Playing ###

uint8_t nowPlayingSize = 2;
uint8_t nowPlayingCounter = 0;

// ### Menu ###

char * menu[] = {
	"Main Menu",
	">Songs",
	">Artists",
	">Now Playing",
	NULL
};
uint8_t menuSize = 4;
uint8_t menuCounter = 1;

// ### Artists ###

// char * artists[] = {
// 	"Artists:",
// 	">Dre",
// 	">Beach Boys",
// 	NULL
// };

char * artists[10] = {};
uint8_t artistCounter = 1;
uint8_t artistSize = 2;

/*	### Notes ###
1. Get artist and song title from the mp3 files 
and store it in the artistStruct (maybe we can 
expand the struct to get album title, length of song, etc.)
2. Make a function to check if artist already exists in the struct.
3. Look into how we mmight be able to dynamically size some of these arrays.


*/
struct artistStruct {
	char *nameOfArtist;
	char * artistSongs[3];		// I set it up to be 3, we should probably make it larger or figure out how to dynamically size it
	int songCount;				// this variable stores how many songs the artist has
};

artistStruct myArtists[4] = {	// I set it up to be 4, it should be larger or we need to use realloc, calloc, or malloc or something
	{"Dre", {"Forgot About Dre", "Fuck You", NULL}, 2},		// these are all just sample entries for the struct
	{"Weezer", {"Say it Ain't So", "", NULL}, 1},			// in a real case, the struct will be filled with what it reads from the mp3
	{"", {"", "", ""}, -1}									// empty struct to indicate end of the array
};
artistStruct *artistStructPtr;	// this is used in some functions below

// ### Songs ###

// char * songs[] = {
// 	"Songs:",
// 	">Forgot About Dre",
// 	">California Girls",
// 	NULL

// };

char * songs[10] = {};
uint8_t songsSize = 4;		// there is actually three songs, but the first entry is the title of the menu
uint8_t songsCounter = 1;

// ### Functions for getting the songs and stuff ###

void getSongsFromStruct()
{
	int songIndex = 1;		// start at one because of previous codes
	int i = 0;
	int artistSongCount = 0;
	while (myArtists[i].songCount != -1)
	{
		while (artistSongCount < myArtists[i].songCount)
		{
			songs[songIndex] = myArtists[i].artistSongs[artistSongCount];
			artistSongCount++;
			songIndex++;
		}
		artistSongCount = 0;
		i++;
	}

	
	printf("%s, %s, %s\n", songs[1], songs[2], songs[3] );
}

void getArtistFromStruct()
{
	int artistIndex = 1;		// start at one because of previous codes
	int i = 0;
	
	while (myArtists[i].songCount != -1)
	{
		artists[artistIndex] = myArtists[i].nameOfArtist;
		i++;
		artistIndex++;
	}

	printf("%s, %s\n", artists[1], artists[2]);
}

void getArtistFromSong()
{
	int songIndex = 1;		// start at one because of previous codes
	int i = 0;
	int artistSongCount = 0;
	while (myArtists[i].songCount != -1)
	{
		while (artistSongCount < myArtists[i].songCount)
		{
			if (strcmp (songPlayingPtr[songsCounter-1], myArtists[i].artistSongs[artistSongCount]) == 0)
			{
				artistStructPtr = &myArtists[i];
				break;
			}
			artistSongCount++;
		}
		artistSongCount = 0;
		i++;
	}
}

typedef enum {
	mainMenuScreen,
	artistsScreen,
	songsScreen,
	nowPlayingScreen
} myStateType;
myStateType currentState = mainMenuScreen;
myStateType previousState = mainMenuScreen;
myStateType tempState = mainMenuScreen;

uint8_t artistsSize = 2;
uint8_t artistsCounter = 1;


void Eint3Handler(void)
{
    IntDriver->HandleInterrupt();
}

void switchPressHandler()
{
    //give semaphore from ISR to waiting task
    //just uart0_puts()
    xSemaphoreGiveFromISR(xSemaphore, NULL);
}

void spiSendString(char *d)
{
	lcd_cs.setLow();
	int i = 0;
	while (d[i] != '\0')
	{
		delay_ms(5);
		instance.transfer(d[i]);
		i++;
	}
	lcd_cs.setHigh();
}

void newLine ()
{
	lcd_cs.setLow();
	instance.transfer(0x0d);
	lcd_cs.setHigh();
}

void clearDisplay()
{
	lcd_cs.setLow();
	instance.transfer(0x7c);
	instance.transfer(0x2d);
	lcd_cs.setHigh();
}

void prepareForNextState()
{
	uart0_puts ("prepare for segue\n");
	switch(currentState)
	{
		case mainMenuScreen:
		{	
			if (menuCounter == 2)
			{
				currentState = songsScreen;
				uart0_puts("cs is now song screen\n");
				break;
			}

			if (menuCounter == 3)
			{
				currentState = artistsScreen;
				uart0_puts("cs is now artisits screen\n");
				break;
			}

			if (menuCounter == 4)
			{
				currentState = nowPlayingScreen;
				uart0_puts("cs is now playing screen\n");
				break;
			}
			
			
		}
		case artistsScreen:
		{
			break;
		}
		case songsScreen:
		{
			songPlayingPtr = songs;
			getArtistFromSong();
			currentState = nowPlayingScreen;
			break;
		}

		case nowPlayingScreen:
		{
			break;
		}
	}

}



void displayCurrentState()
{
	switch (currentState)
	{
		case mainMenuScreen:
		{
			if (menuCounter == 4)
        	{
        		menuCounter = 1;
        	}
        	printf("%d\n", menuCounter);
        	uart0_puts("displaying main menu\n");
            clearDisplay();
            spiSendString(menu[0]);
            newLine();
            spiSendString(menu[menuCounter]);
            menuCounter++;
			break;
		}

		case songsScreen:
		{
			uart0_puts ("displaying song screen\n");
			if (songsCounter == songsSize)
        	{
        		songsCounter = 1;
        	}
            clearDisplay();
            spiSendString("Songs: ");
            newLine();
            spiSendString(songs[songsCounter]);
            songsCounter++;
			break;
		}

		case artistsScreen:
		{
			uart0_puts ("displaying artists screen\n");
			if (songsCounter == 3)
        	{
        		songsCounter = 1;
        	}
            clearDisplay();
            spiSendString(songs[0]);
            newLine();
            spiSendString(songs[songsCounter]);
            songsCounter++;
			break;
		}

		case nowPlayingScreen:
		{
			uart0_puts ("displaying now Playing screen\n");
			if (nowPlayingCounter == nowPlayingSize)
        	{
        		nowPlayingCounter = 0;
        	}
            clearDisplay();
            spiSendString("Now Playing: ");
            newLine();
            if (nowPlayingCounter == 0) spiSendString(artistStructPtr->nameOfArtist);
            else spiSendString(songPlayingPtr[songsCounter-1]);
            
            nowPlayingCounter++;
			break;
		}
	}

}

void vControlLED(void *)
{
    while(1)
    {
    	// uart0_puts("waiting to control leds");
        // if (downButton)
        if (xSemaphoreTake(downSemaphore, 0))
        {
        	downButton = 0;
        	uart0_puts ("down button pressed\n");
        	led3.setLow();
        	displayCurrentState();
        }
        	
        // if (selectButton)
        if (xSemaphoreTake(selectSemaphore, 0))
        {
        	led2.setLow();
        	previousState = currentState;
        	uart0_puts ("select button pressed\n");
        	prepareForNextState();
        	displayCurrentState();
        	selectButton = 0;

        }

        if (xSemaphoreTake(backSemaphore, 0))
        {
        	led4.setLow();
        	uart0_puts ("back button pressed\n");
        	tempState = currentState;
        	currentState = previousState;
        	previousState = tempState;
        	displayCurrentState();
        	selectButton = 0;

        }
        led1.setHigh();
        led2.setHigh();
        led3.setHigh();
        led4.setHigh();
    }
}

void vReadSwitch(void *)
{
	while (1)
	{
		delay_ms(150);
		if (sw1.getLevel())
		{
			uart0_puts("1 pressed\n");
			xSemaphoreGive(xSemaphore);
		}

		if (sw2.getLevel())					// select button
		{
			while (sw2.getLevel())
			{
				uart0_puts("2 pressed\n");		
			}
			xSemaphoreGive(selectSemaphore);
			selectButton = 1;
		}

		if (sw3.getLevel())					// down button
		{
			while (sw3.getLevel()) {
				uart0_puts("3 pressed\n");
				
			}
			xSemaphoreGive(downSemaphore);
			downButton = 1;
		}

		if (sw4.getLevel())
		{
			while (sw4.getLevel())
			{
				uart0_puts("4 pressed\n");
			}
			xSemaphoreGive(backSemaphore);
		}

		// else { uart0_puts ("nothing pressed\n"); }
	}
}

void buttonAndLedSetup()
{
	sw1.setAsInput();
	sw2.setAsInput();
	sw3.setAsInput();
	sw4.setAsInput();

	led1.setAsOutput();
	led2.setAsOutput();
	led3.setAsOutput();
	led4.setAsOutput();

	uart0_puts ("leds and buttons setup");
}

void outputSetup () 
{
    lcd_cs.setAsOutput();
}

void spiLCDInit()
{
    outputSetup();
    instance.initialize(8, MP3SPI::SPI, 128);
    delay_ms(1000);

    lcd_cs.setLow();
    instance.transfer(0x7c);	// setting mode
    instance.transfer(0x2d);	// clearing
    lcd_cs.setHigh();
    
    lcd_cs.setLow();
    instance.transfer(0x7c);
    instance.transfer(0x10);	// baud rate
    lcd_cs.setHigh();

    lcd_cs.setLow();
    instance.transfer(0x7c);	// setting mode
    instance.transfer(0x2d);
    // instance.transfer(0x7c);
    // instance.transfer(0x05);		// trying to set to 4 lines
    lcd_cs.setHigh();

    /*
    lcd_cs.setLow();
    instance.transfer('a');	
    instance.transfer(0x62);
    instance.transfer(0x63);
    instance.transfer(0x0d);		// carriage return command...whichh works as new line
    instance.transfer('n');
    instance.transfer(0x0a);		// line feed..doesn't seem to do new line
    lcd_cs.setHigh();

    char *test = "Song";	
    spiSendString(test);			// this func works
    */
}

int main (void)
{
    // IntDriver->Initialize();
    outputSetup();
    buttonAndLedSetup();
    // menuSize = sizeof(menu)/sizeof(menu[0]);		// get the menu size
    // isr_register(EINT3_IRQn, Eint3Handler);
    char **ptr;		// array of pointers
    ptr = menu;		//pointing to 0th element of menu ### leave this line ###
    // ptr = getCurrentState();
    // statePtr = menu;
    // statePtr = &menu;
    char *(*ptr2)[5];
    ptr2 = &menu;		// points to whole array

    getSongsFromStruct();
    getArtistFromStruct();

    // artistStructPtr = &myArtists[1];
    // printf("%s\n", artistStructPtr->nameOfArtist);

    // songPlayingPtr = songs;
    // printf("%s\n", songPlayingPtr[2]);
   
    xSemaphore = xSemaphoreCreateBinary();
    downSemaphore = xSemaphoreCreateBinary();
    selectSemaphore = xSemaphoreCreateBinary();
    backSemaphore = xSemaphoreCreateBinary();
    delay_ms(50);
    spiLCDInit();

    const uint32_t STACK_SIZE_WORDS = 128;
    xTaskCreate(vControlLED, "vControlLED", STACK_SIZE_WORDS, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vReadSwitch, "vReadSwitch", STACK_SIZE_WORDS, NULL, tskIDLE_PRIORITY + 1, NULL);

    /*
    IntDriver->AttachInterruptHandler(1, 9, switchPressHandler, kFallingEdge);
    IntDriver->AttachInterruptHandler(1, 10, switchPressHandler, kFallingEdge);
    IntDriver->AttachInterruptHandler(1, 14, switchPressHandler, kFallingEdge);
    IntDriver->AttachInterruptHandler(1, 15, switchPressHandler, kFallingEdge);
    */
    vTaskStartScheduler();
    return 0;
}


/*
    char cab[4] = "cab";

    int i = 0;
    int mapCounter = 0;
    while (cab[i] != '\0')
    {
        for (mapCounter = 0; mapCounter < 3; mapCounter++)
        {
            if (cab[i] == my_map[mapCounter].value)
            {
                printf("%d\n", my_map[mapCounter].command);
                mapCounter = 0;
                i++;
                break;
            }

            else {
                printf("not found\n");
            }
        }

    }

    */

/*	### Pointer Fuckery ###
	char **ptr;			// array of pointers
    // ptr = menu;		//pointing to 0th element of menu ### leave this line ###
    ptr = getCurrentState();

    char *(*ptr2)[5];
    ptr2 = &menu;		// points to whole array

    printf("%s\n, %s\n", *ptr, *(ptr+1));		prints array[0] and array[1]
*/

/*
if (menuCounter == 4)
        	{
        		menuCounter = 1;
        	}
            led1.setLow();
            led2.setLow();
            led3.setLow();
            led4.setLow();
            clearDisplay();
            spiSendString(menu[0]);
            newLine();
            spiSendString(menu[menuCounter]);
            menuCounter++;

            vTaskDelay(100);
        }
*/