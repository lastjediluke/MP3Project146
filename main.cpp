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

// Adafruit Write function: drawChar, startWrite, 


#include <stdio.h>
#include "utilities.h"
#include "io.hpp"
#include "MP3SPI.hpp"
#include "MP3GPIO.hpp"



MP3SPI instance;
// MP3GPIO resetPin(0, 0);                 // green
// MP3GPIO lcd_cs(1, 0);                   // blue
// MP3GPIO commandParamControl(0, 2);      // orange
// MP3GPIO ledPower(1, 2);                 // maroon
uint8_t data[4];
int isLEDOn;
int isResetOn;
int rw;
MP3GPIO VO (26, 0);     // orange
MP3GPIO RS (31, 1);     // purple ish
MP3GPIO RW (30, 1);     // green
MP3GPIO E (29, 1);     // blue
MP3GPIO DB0 (0, 2);     // orange
MP3GPIO DB1 (1, 2);     // orange
MP3GPIO DB2 (2, 2);     // orange
MP3GPIO DB3 (3, 2);     // orange
MP3GPIO DB4 (4, 2);     // orange
MP3GPIO DB5 (5, 2);     // orange
MP3GPIO DB6 (6, 2);     // orange
MP3GPIO DB7 (7, 2);     // orange
MP3GPIO BL1 (1, 0);     // orange
MP3GPIO BL2 (0, 0);     // orange




/*spi lcd pin color map:
1. green = reset
2. blue = cs
3. yellow = mosi
4. white = miso
5. gray = sck
6. maroon = led
7. orange = dc/rs
*/



// I'll need to flesh this out further...

struct MapIntToString
{
    char value;
    uint8_t command;
};

MapIntToString my_map[3] = {
    { 'a', 0x90 },
    { 'b', 0x40 },
    { 'c', 0xff }
};

void outputSetup () 
{
    // VO.setAsOutput();
    RS.setAsOutput();
    RW.setAsOutput();
    E.setAsOutput();
    DB0.setAsOutput();
    DB1.setAsOutput();
    DB2.setAsOutput();
    DB3.setAsOutput();
    DB4.setAsOutput();
    DB5.setAsOutput();
    DB6.setAsOutput();
    DB7.setAsOutput();
    // BL1.setAsOutput();
    // BL2.setAsOutput();

}

void write (char byte)
{

    outputSetup(); //makes all the pins as outputs
    if(byte&1<<7)
    {
        DB7.setHigh(); 
    }
    else
    {
        DB7.setLow(); 
    }
    if(byte&1<<6)
    {
        DB6.setHigh(); 
    }
    else
    {
        DB6.setLow();
    }
    if(byte&1<<5)
    {
        DB5.setHigh(); 
    }
    else
    {
        DB5.setLow(); 
    }
    if(byte&1<<4)
    {
        DB4.setHigh(); 
    }
    else
    {
        DB4.setLow(); 
    }
    if(byte&1<<3)
    {
        DB3.setHigh(); 
    }
    else
    {
        DB3.setLow(); 
    }
    if(byte&1<<2)
    {
        DB2.setHigh(); 
    }
    else
    {
        DB2.setLow();
    }
    if(byte&1<<1)
    {
        DB1.setHigh(); 
    }
    else
    {
        DB1.setLow(); 
    }
    if(byte & 1 << 0)
    {
        DB0.setHigh(); 
    }
    else
    {
        DB0.setLow(); 
    }
            
}

void setAllHigh()
{
    RS.setLow();
    RW.setLow();
    E.setHigh();
    DB0.setHigh();
    DB1.setHigh();
    DB2.setHigh();
    DB3.setHigh();
    DB4.setHigh();
    DB5.setHigh();
    DB6.setHigh();
    DB7.setHigh();
}

void setAllLow()
{
    RS.setLow();
    RW.setLow();
    E.setLow();
    DB0.setLow();
    DB1.setLow();
    DB2.setLow();
    DB3.setLow();
    DB4.setLow();
    DB5.setLow();
    DB6.setLow();
    DB7.setHigh();
}


void charScreenInit()
{
   
    E.setLow();
    RS.setLow();
    RW.setLow();
    delay_ms(1000);
   
    E.setHigh();
    delay_ms(6);
    // write(0x1c);
    DB0.setLow();
    DB1.setLow();
    DB2.setLow();
    DB3.setLow();
    DB4.setHigh();
    DB5.setHigh();
    DB6.setLow();
    DB7.setLow();
    delay_ms(6);
    E.setLow();
    delay_ms(6);
    RW.setHigh();

    delay_ms(6);

    RW.setLow();
    delay_ms(1);
    E.setHigh();
    delay_ms(1);
    DB0.setLow();
    DB1.setHigh();
    DB2.setHigh();
    DB3.setHigh();
    DB4.setLow();
    DB5.setLow();
    DB6.setLow();
    DB7.setLow();
    // write(0x0e);
    delay_ms(1);
    E.setLow();
    delay_ms(1);
    RW.setHigh();

    RW.setLow();
    delay_ms(1);
    E.setHigh();
    delay_ms(1);
    write(0x06);
    delay_ms(1);
    E.setLow();
    delay_ms(1);
    RW.setHigh();

    RW.setLow();
    delay_ms(1);
    E.setHigh();
    delay_ms(1);
    write(0x06);
    delay_ms(1);
    E.setLow();
    delay_ms(1);
    RW.setHigh();

}

/*
void lcdScreenInit()
{
    resetPin.setAsOutput();
    lcd_cs.setAsOutput();
    commandParamControl.setAsOutput();
    ledPower.setAsOutput();

    lcd_cs.setHigh();
    ledPower.setLow();
    commandParamControl.setLow();
    resetPin.setHigh();
    resetPin.setLow();
    delay_ms(20);
    resetPin.setHigh();

    lcd_cs.setLow();
    data[0] = instance.transfer(0x11);
    lcd_cs.setHigh();

    delay_ms(61);

    lcd_cs.setLow();
    data[1] = instance.transfer(0x29);
    lcd_cs.setHigh();
    ledPower.setHigh();

    isLEDOn = ledPower.getLevel();
    isResetOn = resetPin.getLevel();
    rw = commandParamControl.getLevel();

    printf("%i, %i, %i\n", isLEDOn, isResetOn, rw);

}

void displayID()
{
    lcd_cs.setLow();
    data[0] = instance.transfer(0xda);
    data[1] = instance.transfer(0xda);
    data[2] = instance.transfer(0xda);
    lcd_cs.setHigh();
}

void inversion()
{
    lcd_cs.setLow();
    instance.transfer(0x21);
    lcd_cs.setHigh();
}

*/

int main (void)
{

    // instance.initialize(8, MP3SPI::SPI, 2);
    // lcdScreenInit();
    // inversion();

    outputSetup();
    delay_ms(50);
    setAllLow();
    // charScreenInit();
    // setAllHigh();

    // int a = DB0.getLevel();
    // int b = DB1.getLevel();
    // int c = DB2.getLevel();
    // int d = DB3.getLevel();
    // int e = DB4.getLevel();
    // int f = DB5.getLevel();
    // int g = DB6.getLevel();
    // int h = DB7.getLevel();
    // int enbale = E.getLevel();
    // int rs = RS.getLevel();
    // int rw = RW.getLevel();

    // printf("%d, %d\n, %d, %d\n, %d, %d\n, %d, %d\n, %d, %d\n, %d\n", a, b, c, d, e, f, g, h, enbale, rs, rw);
    

    

    /*
    char cab[4] = "cab";

    int i = 0;
    int mapCounter = 0;
    while (cab[i] != '\0')
    {
        for (mapCounter = 0; mapCounter < 3; mapCounter++)
        {
            if (cab[i] == my_map[mapCounter].value)
            {
                printf("%d\n", my_map[mapCounter].command);
                mapCounter = 0;
                i++;
                break;
            }

            else {
                printf("not found\n");
            }
        }

    }

    */

    
    
    return 0;
}
