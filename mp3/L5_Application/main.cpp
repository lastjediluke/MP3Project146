// #include "FreeRTOS.h"
// #include "task.h"
// #include <stdio.h>
// #include <string.h>
// #include "printf_lib.h"
// #include "str.hpp"

// #include "Flash.h"
// #include "storage.hpp"

// void vprintMp3Files(void * pvParam) {
// 	while (1) {
// 		vTaskDelay(50);
// 	}
// }

// int main(int argc, char const *argv[]){
// 	char songTitles[10][20];
// 	char artists[10][20];
// 	Flash obj;
// 	obj.mp3_init((char*)songTitles, (char*)artists, (uint8_t)10, (uint8_t)20);

// 	uint8_t numSongs = obj.get_number_of_songs();
// 	for (uint8_t i = 0; i < numSongs; i++) {
// 		printf("Artist: ");
// 		printf(artists[i]);
// 		printf("\n");
// 		printf("Song Title: ");
// 		printf(songTitles[i]);
// 		printf("\n");
// 	}

// 	xTaskCreate(vprintMp3Files, "printMp3Files", 4096, (void*) 0, 1, NULL);

// 	vTaskStartScheduler();

// 	while(1);

// 	return 0;
// }

#include <stdio.h>
#include "utilities.h"
#include "io.hpp"
// #include "MP3SPI.hpp"
// #include "MP3GPIO.hpp"
#include "FreeRTOS.h"
#include "uart0_min.h"
#include "LPC17xx.h"
#include "eint.h"
// #include "INTDrv.hpp"
#include "lpc_isr.h"
// #include "Flash.h"
// #include "labSPI.hpp"
// #include "artist.hpp"
// #include "song.hpp"
// #include "lcd.hpp"
// #include "state.hpp"
// #include "functions.hpp"

#include "ADCDrv.hpp"
#include "Interrupt.h"
// #include "VS1011Drv.hpp"

// INTDrv * IntDriver = INTDrv::instance();

TaskHandle_t viewController;

SemaphoreHandle_t downSemaphore = NULL;
SemaphoreHandle_t upSemaphore = NULL;
SemaphoreHandle_t selectSemaphore = NULL;
SemaphoreHandle_t backSemaphore = NULL;
SemaphoreHandle_t playPauseSemaphore = NULL;
SemaphoreHandle_t volDownSemaphore = NULL;
SemaphoreHandle_t volUpSemaphore = NULL;

bool downButton = 0;
bool upButton = 0;
bool selectButton = 0;
bool backButton = 0;

uint16_t volPot = 0;

Interrupt intr;
ADCDrv adc_driver;

void volUpDown(void *pvParams)
{
	char * result = new char[32];
	long yield = 0;
	while (1)
	{
		uart0_puts("inside volUpDown if\n");
		float tmp = adc_driver.ReadAdcVoltageByChannel(3);
		sprintf(result, "%f" , tmp);
        uart0_puts(result);
		if (tmp > volPot)
		{
			volPot = tmp;
			uart0_puts("inside volume up\n");
			uint16_t incr = ((incr / 0.5) << 8) + (incr / 0.5);
			result -= incr;
			decoder.SendSCIWriteCommand(0xB, result);
		}
		else if (tmp < volPot)
		{
			volPot = tmp;
			uart0_puts("inside volume down\n");
			uint16_t incr = ((incr / 0.5) << 8) + (incr / 0.5);
			result -= incr;
			decoder.SendSCIWriteCommand(0xB, result);
		}
		else
		{
			volPot = tmp;
			uart0_puts("malfunction\n");
			uint16_t incr = ((incr / 0.5) << 8) + (incr / 0.5);
			result -= incr;
			decoder.SendSCIWriteCommand(0xB, result);
		}
		vTaskDelay(500);
	}
}

// actual volume ranges from 0 to 254dB. If volume reaches 255dB, it triggers powerdown mode
void volUp()
{
	long yield = 0;
	xSemaphoreGiveFromISR(volUpSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

	if (xSemaphoreTake(volUpSemaphore, portMAX_DELAY))
    {
		uart0_puts("inside volume up\n");
		// uint16_t incr = 0x3232;
		// VS1011Drv decoder;
        // uint16_t result = decoder.SendSCIReadCommand(0xB);
        // incr = ((incr / 0.5) << 8) + (incr / 0.5);
        // result -= incr;
        // decoder.SendSCIWriteCommand(0xB, result);
    }
}

void volDown() 
{
    long yield = 0;
	xSemaphoreGiveFromISR(volDownSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

	if (xSemaphoreTake(volDownSemaphore, portMAX_DELAY))
    {
		uart0_puts("inside volume up\n");
		// uint16_t incr = 0x3232;
		// VS1011Drv decoder;
        // uint16_t result = decoder.SendSCIReadCommand(0xB);
        // incr = ((incr / 0.5) << 8) + (incr / 0.5);
        // result += incr;
        // decoder.SendSCIWriteCommand(0xB, result);
    }
}

void playOrPause()
{
	long yield = 0;
	xSemaphoreGiveFromISR(playPauseSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

	if (xSemaphoreTake(playPauseSemaphore, portMAX_DELAY))
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
}

void back() 
{
	long yield = 0;
	xSemaphoreGiveFromISR(backSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

	if (xSemaphoreTake(backSemaphore, portMAX_DELAY))
    {
		// led4.setLow();
		uart0_puts ("back button pressed\n");
		// currentState = mainMenuScreen;
		// displayCurrentState();
		// selectButton = 0;
    }
}

void select()
{
	long yield = 0;
	xSemaphoreGiveFromISR(selectSemaphore, &yield);
	portYIELD_FROM_ISR(yield);	

	if (xSemaphoreTake(selectSemaphore, portMAX_DELAY))
    {
		// led2.setLow();
		uart0_puts ("select button pressed\n");
		// prepareForNextState();
		// displayCurrentState();
		// selectButton = 0;
    }
}

void down()
{
	long yield = 0;
	xSemaphoreGiveFromISR(downSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

	if (xSemaphoreTake(downSemaphore, portMAX_DELAY))
	{
		// downButton = 0;
		uart0_puts ("down button pressed\n");
		// led3.setLow();
		// displayCurrentState();
	}
}

void up()
{
	long yield = 0;
	xSemaphoreGiveFromISR(upSemaphore, &yield);
	portYIELD_FROM_ISR(yield);

	if (xSemaphoreTake(upSemaphore, portMAX_DELAY))
	{
		// upButton = 0;
		uart0_puts ("up button pressed\n");
		// led3.setLow();
		// displayCurrentState();
	}
}

void Eint3Handler(void)
{
	intr.HandleInterrupt();
}

void ADCHandler(void)
{
	intr.HandleADCInterrupt();
}

int main (void)
{
	// VS1011Drv decoder;
    // volPot = decoder.SendSCIReadCommand(0xB);

    adc_driver.AdcInitBurstMode();
    adc_driver.AdcSelectPin(ADCDrv::k0_26);

	uart0_puts ("inside main\n");

	// create interrupt buttons for volume up and down
    intr.Initialize();
    // when interrupt is triggered, go to Eint3Handler()
    isr_register(EINT3_IRQn, Eint3Handler);		// button interrupt

	// need to define port and pin numbers
    // intr.AttachInterruptHandler(2, 0, volUp, kRisingEdge);
    // intr.AttachInterruptHandler(0, 0, volDown, kRisingEdge);
	// intr.AttachInterruptHandler(2, 6, playOrPause, kRisingEdge);
	// intr.AttachInterruptHandler(2, 7, back, kRisingEdge);
	// intr.AttachInterruptHandler(0, 1, select, kRisingEdge);
	// intr.AttachInterruptHandler(0, 0, down, kRisingEdge);

	// intr.AttachInterruptHandler(0, 0, up, kRisingEdge);

    // outputSetup();
    // buttonAndLedSetup();

    // getSongsFromStruct();
    // getArtistFromStruct();

    downSemaphore = xSemaphoreCreateBinary();
	upSemaphore = xSemaphoreCreateBinary();
    selectSemaphore = xSemaphoreCreateBinary();
    backSemaphore = xSemaphoreCreateBinary();
	playPauseSemaphore = xSemaphoreCreateBinary();
	volDownSemaphore = xSemaphoreCreateBinary();
	volUpSemaphore = xSemaphoreCreateBinary();

	xTaskCreate(volUpDown, "volUpDown", 1024, (void*) 0, 1, NULL);
	vTaskStartScheduler();

	while(1){}
    return 0;
}