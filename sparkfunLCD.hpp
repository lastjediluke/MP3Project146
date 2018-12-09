#ifndef SPARKFUNLCD_HPP__
#define SPARKFUNLCD_HPP__

#include <stdio.h>
#include <stddef.h>
#include "utilities.h"
#include "io.hpp"
#include "MP3GPIO.hpp"
#include "FreeRTOS.h"
#include "tasks.hpp"
#include "task.h"
#include "uart0_min.h"
#include "LPC17xx.h"
#include <string.h>
#include "Flash.h"
#include "labSPI.hpp"
// #include "buttonsAndGlobals.hpp"



class sparkfunLCD 
{
    public:
        sparkfunLCD();
        ~sparkfunLCD();

        struct artistStruct 
        {
            char *nameOfArtist;
            char * artistSongs[3];		
            int songCount;				
        };

        void buttonAndLedSetup();

        void spiLCDInit();

        void spiSendString(char *d);

        void newLine();

        void clearDisplay();

        void resetTimer();

        void prepareForNextState();

        void displayCurrentState();

        void getSongsFromStruct();

        void getArtistFromStruct();

        void getArtistFromSong();

        void mp3ToStruct();

        void classInit();

        MP3GPIO lcd_cs;  // lcd_cs(1, 0);                 

        MP3GPIO sw1; //  (9, 1);		
        MP3GPIO sw2;       // (10, 1);		
        MP3GPIO sw3;       // (14, 1);		
        MP3GPIO sw4;    // (15, 1);

        MP3GPIO led1; // (0, 1);
        MP3GPIO led2;   // (1, 1);
        MP3GPIO led3;   // (4, 1);
        MP3GPIO led4;       // (8, 1);

        bool songIsPlaying;

        typedef enum 
        {
            mainMenuScreen,
            artistsScreen,
            songsScreen,
            nowPlayingScreen
        } myStateType;
        myStateType currentState = mainMenuScreen;
        myStateType previousState = mainMenuScreen;
        myStateType tempState = mainMenuScreen;

    private:

        
        char **songPlayingPtr;
        int seconds;
        int minute;
        int bytes2write;
        char data[20];

        // char * menu[5];
        uint8_t menuSize;
        uint8_t menuCounter;

        int nowPlayingSize;		
        int nowPlaying;
        int nowPlayingCounter;

        char * artists[10] = {};
        uint8_t artistCounter;
        uint8_t artistSize;

        // artistStruct myArtists[4];
        artistStruct fromMP3ArtistStruct[3];
        artistStruct *artistStructPtr;

        char * songs[10] = {};
        int songsSize;		
        int songsCounter;

        LabSPI instance;
        

        char *menu[5] = 
        {
            "Main Menu",
            ">Songs",
            ">Artists",
            ">Now Playing",
            NULL
        };

        artistStruct myArtists[4] = 
        {	// I set it up to be 4, it should be larger or we need to use realloc, calloc, or malloc or something
            {"Dre", {"Forgot About Dre", "Fuck You", NULL}, 2},		// these are all just sample entries for the struct
            {"Weezer", {"Say it Ain't So", "", NULL}, 1},			// in a real case, the struct will be filled with what it reads from the mp3
            {"", {"", "", ""}, -1}									// empty struct to indicate end of the array
        };

        

           
};

#endif