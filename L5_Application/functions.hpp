#ifndef FUNCTIONS_HPP__
#define FUNCTIONS_HPP__

#include <stdio.h>
#include "utilities.h"
#include "uart0_min.h"
#include "LPC17xx.h"
#include <string.h>
#include "Flash.h"
#include "labSPI.hpp"
#include "artist.hpp"
#include "song.hpp"
// #include "state.hpp"


/*

void finalCountdown()
{
	int seconds = 0;
	int minute = 0;

	while (1) 
	{
		if ( seconds == 60 )
		{
			minute++;
			seconds = 0;

		}
		printf("M: %d, S: %d\n", minute, seconds );
		delay_ms(1000);
		seconds++;
	}
}
*/

char **songPlayingPtr = NULL;

// ### Now Playing ###

int nowPlayingSize = 3;		// this will be the size of how ever much info we decide to display
// in this case, we are only displaying album and artist
int nowPlaying = 0;
int nowPlayingCounter = 0;


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

void mp3ToStruct()
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

// char *nameOfArtist;
// char * artistSongs[3];		// I set it up to be 3, we should probably make it larger or figure out how to dynamically size it
// int songCount;


#endif