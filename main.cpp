/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * @brief This is the application entry point.
 */

#include <stdio.h>
#include "utilities.h"
#include "io.hpp"
#include "sparkfunLCD.hpp"
#include "FreeRTOS.h"
#include "tasks.hpp"
#include "task.h"
#include "uart0_min.h"
#include "eint.h"
#include "lpc_isr.h"
// #include "Interrupt.h"
#include "INTDrv.hpp"

TaskHandle_t viewController;

// Interrupt intr;
INTDrv * intr = INTDrv::instance();

SemaphoreHandle_t downSemaphore = NULL;
SemaphoreHandle_t selectSemaphore = NULL;
SemaphoreHandle_t backSemaphore = NULL;
SemaphoreHandle_t xSemaphore = NULL;

bool downButton = 0;
bool selectButton = 0;
bool backButton = 0;

char **statePtr;

sparkfunLCD myLCD;



// menu, songs, artists were here

// now playing and menu and song pointer were here

// ### Functions for getting the songs and stuff ###

// functions were here

// screen states typedef was here


// ### interrupt stuff that isn't really being used


// states was here

// ### tilt task ###

// ### Responding to button presses ###

void vControlLED(void *)
{
	
	int ticksToWait = ( TickType_t ) 10;
    while(1)
    {
    	// uart0_puts("waiting to control leds");
        // if (downButton)
        if (xSemaphoreTake(downSemaphore, ticksToWait))
        {
        	downButton = 0;
        	uart0_puts ("down button pressed\n");
        	myLCD.led3.setLow();
        	myLCD.displayCurrentState();
        	// vTaskSuspend( NULL );

        }
        	
        // if (selectButton)
        if (xSemaphoreTake(selectSemaphore, ticksToWait))
        {
        	myLCD.led2.setLow();
        	// previousState = currentState;
        	uart0_puts ("select button pressed\n");
        	myLCD.prepareForNextState();
        	myLCD.displayCurrentState();
        	selectButton = 0;
        	// vTaskSuspend( NULL );

        }

        if (xSemaphoreTake(backSemaphore, ticksToWait))
        {
        	myLCD.led4.setLow();
        	uart0_puts ("back button pressed\n");
        	// getWhereToGoBackTo();
        	// tempState = currentState;
        	myLCD.currentState = myLCD.mainMenuScreen;
        	// previousState = tempState;
        	myLCD.displayCurrentState();
        	selectButton = 0;
        	// vTaskSuspend( NULL );

        }

        if (xSemaphoreTake(xSemaphore, ticksToWait))
        {
        	
        	if (myLCD.songIsPlaying) {
        		myLCD.songIsPlaying = 0;
        		myLCD.led1.setHigh();
        	}
        	else {
        		myLCD.songIsPlaying = 1;
        		myLCD.led1.setLow();
        	}
        }
        
        // myLCD.led1.setHigh();
        myLCD.led2.setHigh();
        myLCD.led3.setHigh();
        myLCD.led4.setHigh();
        
        
    }
}

// void vReadSwitch(void *)
// {
// 	while (1)
// 	{
// 		delay_ms(150);
// 		if (myLCD.sw1.getLevel())
// 		{
// 			uart0_puts("1 pressed\n");
// 			while (myLCD.sw1.getLevel()){

// 			}
			
// 			xSemaphoreGive(xSemaphore);
// 		}

// 		if (myLCD.sw2.getLevel())					// select button
// 		{
			
// 			while (myLCD.sw2.getLevel())
// 			{
// 				// vTaskResume (viewController);
// 				uart0_puts("2 pressed\n");		
// 			}
// 			// delay_ms(100);
// 			xSemaphoreGive(selectSemaphore);
// 			selectButton = 1;
// 		}

// 		if (myLCD.sw3.getLevel())					// down button
// 		{
			
// 			while (myLCD.sw3.getLevel()) {
// 				// vTaskResume (viewController);
// 				uart0_puts("3 pressed\n");
				
// 			}
// 			// delay_ms(100);
// 			xSemaphoreGive(downSemaphore);
// 			downButton = 1;
// 		}

// 		if (myLCD.sw4.getLevel())
// 		{
			
// 			while (myLCD.sw4.getLevel())
// 			{	
// 				// vTaskResume (viewController);
// 				uart0_puts("4 pressed\n");
// 			}
// 			// delay_ms(100);
// 			xSemaphoreGive(backSemaphore);
// 		}

// 		// else { uart0_puts ("nothing pressed\n"); }
// 	}
// }

// bytes2write = sprintf(myLCD.data, "%d %s\n", tyme, message);
/*

void timerTask(void *)
{
	
	
	while (1)
	{
		
		
			if (myLCD.songIsPlaying)		
			{
				// continue counting
				if ( myLCD.seconds == 60 )
				{
					myLCD.minute++;
					myLCD.seconds = 0;

				}
				// bytes2write = sprintf(myLCD.data, "%d %s\n", tyme, message);
				sprintf(myLCD.data, "M:%d, S:%d\n\0", myLCD.minute, myLCD.seconds);
				// printf("M: %d, S: %d\n", myLCD.minute, myLCD.seconds );
				printf("%s\n", myLCD.data);
				
				myLCD.seconds++;

				if (myLCD.currentState == myLCD.nowPlayingScreen && myLCD.nowPlayingCounter == 3)
				{
					clearDisplay();
					
					spiSendString(myLCD.data);
					
				}
				// myLCD.songIsPlaying = 1;
				vTaskDelay(1000);				
			}

			else 
			{
				if (currentState == nowPlayingScreen && nowPlayingCounter == 3)
				{
					clearDisplay();
					
					spiSendString(myLCD.data);
					vTaskDelay(1000);
					
				}
			}
		
		
	}
}
*/

void playOrPause()
{
	long yield = 0;
	xSemaphoreGiveFromISR(xSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

    /*
	if (xSemaphoreTake(xSemaphore, portMAX_DELAY))
    {
		uart0_puts("play or pause\n");
		// if (songIsPlaying) {
		// 	songIsPlaying = 0;
		// 	led1.setHigh();
		// }
		// else {
		// 	songIsPlaying = 1;
		// 	led1.setLow();
		// }
    }
    */
}

void back() 
{
	long yield = 0;
	xSemaphoreGiveFromISR(backSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

    /*
	if (xSemaphoreTake(backSemaphore, portMAX_DELAY))
    {
		// led4.setLow();
		uart0_puts ("back button pressed\n");
		// currentState = mainMenuScreen;
		// displayCurrentState();
		// selectButton = 0;
    }
    */
}

void select()
{
	long yield = 0;
	xSemaphoreGiveFromISR(selectSemaphore, &yield);
	portYIELD_FROM_ISR(yield);	

    /*
	if (xSemaphoreTake(selectSemaphore, portMAX_DELAY))
    {
		// led2.setLow();
		uart0_puts ("select button pressed\n");
		// prepareForNextState();
		// displayCurrentState();
		// selectButton = 0;
    }
    */
}

void down()
{
	long yield = 0;
	xSemaphoreGiveFromISR(downSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

    /*
	if (xSemaphoreTake(downSemaphore, portMAX_DELAY))
	{
		// downButton = 0;
		uart0_puts ("down button pressed\n");
		// led3.setLow();
		// displayCurrentState();
	}
    */
}

// void up()
// {
// 	long yield = 0;
// 	xSemaphoreGiveFromISR(upSemaphore, &yield);
// 	portYIELD_FROM_ISR(yield);

// 	if (xSemaphoreTake(upSemaphore, portMAX_DELAY))
// 	{
// 		// upButton = 0;
// 		uart0_puts ("up button pressed\n");
// 		// led3.setLow();
// 		// displayCurrentState();
// 	}
// }

void Eint3Handler(void)
{
	intr->HandleInterrupt();
}


int main (void)
{
    printf("begin main\n");
    

    myLCD.classInit();
    myLCD.getSongsFromStruct();
    myLCD.getArtistFromStruct();
    myLCD.spiLCDInit();
    myLCD.led1.setHigh();
    xSemaphore = xSemaphoreCreateBinary();
    downSemaphore = xSemaphoreCreateBinary();
    selectSemaphore = xSemaphoreCreateBinary();
    backSemaphore = xSemaphoreCreateBinary();
    intr->Initialize();
    // when interrupt is triggered, go to Eint3Handler()
    isr_register(EINT3_IRQn, Eint3Handler);

	// need to define port and pin numbers
    // intr.AttachInterruptHandler(2, 0, volUp, kRisingEdge);
    // intr.AttachInterruptHandler(0, 0, volDown, kRisingEdge);
	intr->AttachInterruptHandler(2, 0, playOrPause, kFallingEdge);
	intr->AttachInterruptHandler(2, 1, select, kFallingEdge);
	intr->AttachInterruptHandler(2, 2, down, kFallingEdge);
	intr->AttachInterruptHandler(2, 3, back, kFallingEdge);
    delay_ms(50);
    printf("end main before scheduler\n");
    xTaskCreate(vControlLED, "vControlLED", 1024, NULL, tskIDLE_PRIORITY + 1,  &viewController);
    // xTaskCreate(vReadSwitch, "vReadSwitch", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
    // xTaskCreate(tiltController, "tiltController", 1024, NULL, tskIDLE_PRIORITY + 1, &xHandle);
    // xTaskCreate(timerTask, "timerTask", 128, NULL, tskIDLE_PRIORITY + 1, NULL);
    
    vTaskStartScheduler();
    
    
    return 0;
}
