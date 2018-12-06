#ifndef ARTIST_HPP__
#define ARTIST_HPP__

#include <stdint.h>
#include <stdio.h>
#include "LPC17xx.h"
#include "utilities.h"
#include <stddef.h>
#include "FreeRTOS.h"
#include "tasks.hpp"
#include "uart0_min.h"
#include "io.hpp"
#include "switches.hpp"
#include "gpio.hpp"

// ### Artists ###

// char * artists[] = {
// 	">Dre",
// 	">Beach Boys",
// 	NULL
// };

char * artists[10] = {};
uint8_t artistCounter = 0;
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

// artistStruct myArtists[4] = {	// I set it up to be 4, it should be larger or we need to use realloc, calloc, or malloc or something
	
// 	{"", {"", "", ""}, -1}									// empty struct to indicate end of the array
// };

artistStruct *artistStructPtr;	// this is used in some functions below




#endif