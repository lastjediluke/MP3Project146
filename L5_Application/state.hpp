#ifndef STATE_HPP__
#define STATE_HPP__

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
#include "lpc_isr.h"
#include <string.h>
#include "Flash.h"
#include "labSPI.hpp"
#include "artist.hpp"
#include "song.hpp"

TaskHandle_t xHandle;

char **songPlayingPtr = NULL;



// ### Now Playing ###

uint8_t nowPlayingSize = 2;		// this will be the size of how ever much info we decide to display
// in this case, we are only displaying album and artist
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

// ### screen states ###

typedef enum {
	mainMenuScreen,
	artistsScreen,
	songsScreen,
	nowPlayingScreen
} myStateType;
myStateType currentState = mainMenuScreen;
myStateType previousState = mainMenuScreen;
myStateType tempState = mainMenuScreen;

// ### next state logic ###

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

		case songsScreen:		// when i press select, songCounter is one ahead of where I want it
		{
			songPlayingPtr = songs;
			if ( songsCounter == -1)
			{
				nowPlayingCounter = songsSize - 1;		// end of songs array
			}

			else
			{
				nowPlayingCounter = songsCounter - 1;
			}
			
			// getArtistFromSong();
			vTaskResume(xHandle);
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

// ### usually used for down button presses ###

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
        		songsCounter = 0;
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
            // if (nowPlayingCounter == 0) spiSendString(artistStructPtr->nameOfArtist);
            else spiSendString(songs[nowPlayingCounter-1]);
            
            nowPlayingCounter++;
			break;
		}
	}

}

void getWhereToGoBackTo()
{
	switch (currentState)
	{
		case mainMenuScreen:
		{	
			previousState = currentState;
			currentState = currentState;
			break;
		}
		case songsScreen:
		{
			previousState = currentState;
			currentState = mainMenuScreen;
			break;
		}
		case artistsScreen:
		{
			previousState = currentState;
			currentState = mainMenuScreen;
			break;
		}
		case nowPlayingScreen:
		{
			currentState = previousState;
			break;
		}
	}
}


#endif