#ifndef STATE_HPP__
#define STATE_HPP__

#include <stdio.h>
#include "utilities.h"
#include "io.hpp"
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
#include "functions.hpp"

TaskHandle_t xHandle;

// songpntr was here
bool songIsPlaying = 0;

int seconds = 0;
int minute = 0;
int bytes2write = 0;
char data[20];



// now playing was here

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

void resetTimer()
{
	seconds = 0;
	minute = 0;
}

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

void getWhereToGoBackTo()
{
	switch (currentState)
	{
		case mainMenuScreen:
		{	
			previousState = mainMenuScreen;
			currentState = currentState;
			break;
		}
		case songsScreen:
		{
			previousState = songsScreen;
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