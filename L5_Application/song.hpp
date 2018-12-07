#ifndef SONG_HPP__
#define SONG_HPP__

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

// ### Songs ###

// char * songs[] = {
// 	">Forgot About Dre",
// 	">California Girls",
// 	NULL

// };

char * songs[10] = {};
int songsSize = 3;		// there is actually three songs, but the first entry is the title of the menu
int songsCounter = 0;



#endif