#ifndef LCD_HPP__
#define LCD_HPP__

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

#include "Interrupt.h"

// MP3SPI instance;
LabSPI instance;
MP3GPIO lcd_cs(1, 0);                 

MP3GPIO sw1 (9, 1);		
MP3GPIO sw2 (10, 1);		
MP3GPIO sw3 (14, 1);		
MP3GPIO sw4 (15, 1);

MP3GPIO led1 (0, 1);
MP3GPIO led2 (1, 1);
MP3GPIO led3 (4, 1);
MP3GPIO led4 (8, 1);

void buttonAndLedSetup()
{
	sw1.setAsInput();
	sw2.setAsInput();
	sw3.setAsInput();
	sw4.setAsInput();

	led1.setAsOutput();
	led2.setAsOutput();
	led3.setAsOutput();
	led4.setAsOutput();

	uart0_puts ("leds and buttons setup");
}

void outputSetup () 
{
    lcd_cs.setAsOutput();
}

void Eint3Handler(void){
	intr.HandleInterrupt();
}

void playOrPause() 
{
    
}
void backBtn() 
{

}
void downBtn() 
{

}
void upBtn() 
{

}
void selectBtn()
{

}

void spiLCDInit()
{
    // create interrupt for buttons
    Interrupt intr;

    // choose port and pin for buttons
    intr.AttachInterruptHandler(2, 0, playOrPause, kRisingEdge);	
    outputSetup();
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

void spiSendString(char *d)
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

void newLine ()
{
	lcd_cs.setLow();
	instance.transferMP3(0x0d);
	lcd_cs.setHigh();
}

void clearDisplay()
{
	lcd_cs.setLow();
	instance.transferMP3(0x7c);
	instance.transferMP3(0x2d);
	lcd_cs.setHigh();
}
#endif