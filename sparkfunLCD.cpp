#include "sparkfunLCD.hpp"
#include "MP3GPIO.hpp"

sparkfunLCD::sparkfunLCD()
{
    uart0_puts("begin constructor\n");
    lcd_cs.initialize(1, 0);
    sw1.initialize(9, 1);
    sw2.initialize(10, 1);
    sw3.initialize(14, 1);
    sw4.initialize(15, 1);
    led1.initialize(0, 1);
    led2.initialize(1,1);
    led3.initialize(4,1);
    led4.initialize(8,1);

    uart0_puts("buttons and leds initialized\n");

    songIsPlaying = 0;
    // **songPlayingPtr = NULL;
    seconds = 0;
    minute = 0;
    bytes2write = 0;
    
    uart0_puts("made it past songPlayingPtr = Null\n");


    menuSize = 4;
    menuCounter = 1;

    nowPlayingSize = 3;	
    nowPlaying = 0;
    nowPlayingCounter = 0;

    // *artists[10] = {};
    artistCounter = 0;
    artistSize = 2;

    uart0_puts("made it past artist pointer\n");

    fromMP3ArtistStruct[3] = {};
    // *artistStructPtr = NULL;

    // *songs[10] = {};        // watch it 
    songsSize = 3;		
    songsCounter = 0;
    uart0_puts("constructor\n");
}

void sparkfunLCD::classInit()
{
    uart0_puts("class init\n");
    buttonAndLedSetup();
    // instance.initializeDecoderSPI();

    led1.setLow();
    led2.setLow();
    led3.setHigh();
    led4.setLow();

}

void sparkfunLCD::buttonAndLedSetup()
{
    sw1.setAsInput();
	sw2.setAsInput();
	sw3.setAsInput();
	sw4.setAsInput();

	led1.setAsOutput();
	led2.setAsOutput();
	led3.setAsOutput();
	led4.setAsOutput();

	lcd_cs.setAsOutput();
    uart0_puts("button and led setup\n");
}

void sparkfunLCD::spiLCDInit()
{
    // outputSetup();
    // instance.initialize(8, MP3SPI::SPI, 128);
	instance.initializeDecoderSPI();
    delay_ms(1000);

    lcd_cs.setLow();
    instance.transferMP3(0x7c);	// setting mode
    instance.transferMP3(0x2d);	// clearing
    lcd_cs.setHigh();
    
    lcd_cs.setLow();
    instance.transferMP3(0x7c);
    instance.transferMP3(0x10);	// baud rate
    lcd_cs.setHigh();

    

    delay_ms(2000);
}

void sparkfunLCD::spiSendString(char *d)
{
    lcd_cs.setLow();
	int i = 0;
	while (d[i] != '\0')
	{
		delay_ms(5);
		instance.transferMP3(d[i]);
		i++;
	}
	lcd_cs.setHigh();
}

void sparkfunLCD::newLine()
{
    lcd_cs.setLow();
	instance.transferMP3(0x0d);
	lcd_cs.setHigh();
}

void sparkfunLCD::clearDisplay()
{
    lcd_cs.setLow();
	instance.transferMP3(0x7c);
	instance.transferMP3(0x2d);
	lcd_cs.setHigh();
}

void sparkfunLCD::resetTimer()
{
    seconds = 0;
	minute = 0;
}

void sparkfunLCD::prepareForNextState()
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
				previousState = mainMenuScreen;
				currentState = nowPlayingScreen;
				uart0_puts("cs is now playing screen\n");
				break;
			}
			
			
		}

		case artistsScreen:
		{
			break;
		}

		case songsScreen:		// when i press select, songCounter is one ahead of where I want it
		{
			songPlayingPtr = songs;
			nowPlaying = songsCounter - 1;
			songIsPlaying = 1;
			resetTimer();
			printf("now playing counter is at %d\n", nowPlaying );
			
			// getArtistFromSong();
			// vTaskResume(xHandle);
			currentState = nowPlayingScreen;
			break;
		}

		case nowPlayingScreen:
		{
			currentState = currentState;
			break;
		}
	}
}

void sparkfunLCD::displayCurrentState()
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
			previousState = mainMenuScreen;
			uart0_puts ("displaying song screen\n");
			if (songsCounter == songsSize)
        	{
        		songsCounter = 0;
        	}

        	printf("song counter is at %d\n", songsCounter);
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
			if (artistCounter == artistSize)
        	{
        		artistCounter = 0;
        	}
            clearDisplay();
            spiSendString("Artists: ");
            newLine();
            spiSendString(artists[artistCounter]);
            artistCounter++;
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
            if (songPlayingPtr == NULL) break;
            if (nowPlayingCounter == 0) spiSendString(songs[nowPlaying]);
            if (nowPlayingCounter == 1)
            {
            	getArtistFromSong();
            	spiSendString(artistStructPtr->nameOfArtist);
            }
            // else spiSendString(songs[nowPlaying]);
            
            
            nowPlayingCounter++;
			break;
		}
	}
}

void sparkfunLCD::getSongsFromStruct()
{
    int songIndex = 0;		// start at one because of previous codes
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

	
	printf("%s, %s, %s\n", songs[0], songs[1], songs[2] );
}

void sparkfunLCD::getArtistFromStruct()
{
    int artistIndex = 0;		// start at one because of previous codes
	int i = 0;
	
	while (myArtists[i].songCount != -1)
	{
		artists[artistIndex] = myArtists[i].nameOfArtist;
		i++;
		artistIndex++;
	}

	printf("%s, %s\n", artists[0], artists[1]);
}

void sparkfunLCD::getArtistFromSong()
{
    int i = 0;
	int artistSongCount = 0;
	while (myArtists[i].songCount != -1)
	{
		while (artistSongCount < myArtists[i].songCount)
		{
			if (strcmp (songPlayingPtr[nowPlaying], myArtists[i].artistSongs[artistSongCount]) == 0)
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

void sparkfunLCD::mp3ToStruct()
{
    char songTitles[10][20];
	char artists[10][20];
	Flash obj;
	obj.mp3_init((char*)songTitles, (char*)artists, (uint8_t)10, (uint8_t)20);

	uint8_t numSongs = obj.get_number_of_songs();
	for (uint8_t i = 0; i < numSongs; i++) {
		fromMP3ArtistStruct[i].nameOfArtist = artists[i];
		fromMP3ArtistStruct[i].artistSongs[i] = songTitles[i];
		fromMP3ArtistStruct[i].songCount++;
	}
}

sparkfunLCD::~sparkfunLCD()
{

}

