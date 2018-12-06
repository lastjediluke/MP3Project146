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
#include "Flash.h"
#include "labSPI.hpp"
#include "artist.hpp"
#include "song.hpp"
#include "lcd.hpp"
#include "state.hpp"
#include "functions.hpp"
// #include "testForMP3.cpp"


INTDrv * IntDriver = INTDrv::instance();


// TaskHandle_t xHandle;

SemaphoreHandle_t downSemaphore = NULL;
SemaphoreHandle_t selectSemaphore = NULL;
SemaphoreHandle_t backSemaphore = NULL;
SemaphoreHandle_t xSemaphore = NULL;

bool downButton = 0;
bool selectButton = 0;
bool backButton = 0;

char **statePtr;

// menu, songs, artists were here

// now playing and menu and song pointer were here

// ### Functions for getting the songs and stuff ###

// functions were here

// screen states typedef was here


// ### interrupt stuff that isn't really being used

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

// states was here

// ### tilt task ###

void tiltController ( void * )
{
	uart0_puts("we entered tilt controller\n");
	while ( 1 )
	{
		int tilt_x = AS.getX();
		int tilt_y = AS.getY();
		if (currentState == nowPlayingScreen)
		{	

			
			if (tilt_x > 500)		// left tilt
			{
				uart0_puts("tilt left\n");
				printf("%d\n", songsCounter );
				songsCounter--;
				printf("%d\n", songsCounter );

	        	if (songsCounter <= -1)
	        	{
	        		songsCounter = 2;
	        		printf("%d\n", songsCounter );
	        	}
				clearDisplay();
            	spiSendString("Now Playing: ");
            	newLine();
            	spiSendString(songs[songsCounter]);		// got rid of minus 1
				delay_ms(1000);
			}

			if (tilt_x < -500)		// right tilt
			{

				uart0_puts("tilt right\n");
				songsCounter++;
				if (songsCounter >= songsSize)
	        	{
	        		songsCounter = 0;
	        	}
				clearDisplay();
            	spiSendString("Now Playing: ");
            	newLine();
            	spiSendString(songs[songsCounter]);
				delay_ms(1000);
			}
		}

		else 
		{
			//suspend task

			vTaskSuspend( NULL );
			uart0_puts("task suspended\n");
		}
	}
}

// ### Responding to button presses ###

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
        	// previousState = currentState;
        	uart0_puts ("select button pressed\n");
        	prepareForNextState();
        	displayCurrentState();
        	selectButton = 0;

        }

        if (xSemaphoreTake(backSemaphore, 0))
        {
        	led4.setLow();
        	uart0_puts ("back button pressed\n");
        	getWhereToGoBackTo();
        	// tempState = currentState;
        	// currentState = previousState;
        	// previousState = tempState;
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

int main (void)
{
    // IntDriver->Initialize();
    outputSetup();
    buttonAndLedSetup();
    // menuSize = sizeof(menu)/sizeof(menu[0]);		// get the menu size
    // isr_register(EINT3_IRQn, Eint3Handler);
    // char **ptr;		// array of pointers
    // ptr = menu;		//pointing to 0th element of menu ### leave this line ###
    // ptr = getCurrentState();
    // statePtr = menu;
    // statePtr = &menu;
    // char *(*ptr2)[5];
    // ptr2 = &menu;		// points to whole array

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
    xTaskCreate(vReadSwitch, "vReadSwitch", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(tiltController, "tiltController", 1024, NULL, tskIDLE_PRIORITY + 1, &xHandle);

    /*
    IntDriver->AttachInterruptHandler(1, 9, switchPressHandler, kFallingEdge);
    IntDriver->AttachInterruptHandler(1, 10, switchPressHandler, kFallingEdge);
    IntDriver->AttachInterruptHandler(1, 14, switchPressHandler, kFallingEdge);
    IntDriver->AttachInterruptHandler(1, 15, switchPressHandler, kFallingEdge);
    */
    vTaskStartScheduler();
    return 0;
}

/*	### Pointer Fuckery ###
	char **ptr;			// array of pointers
    // ptr = menu;		//pointing to 0th element of menu ### leave this line ###
    ptr = getCurrentState();

    char *(*ptr2)[5];
    ptr2 = &menu;		// points to whole array

    printf("%s\n, %s\n", *ptr, *(ptr+1));		prints array[0] and array[1]
*/
