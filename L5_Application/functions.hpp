#ifndef FUNCTIONS_HPP__
#define FUNCTIONS_HPP__

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







void getSongsFromStruct()
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

void getArtistFromStruct()
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

void getArtistFromSong()
{
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


#endif