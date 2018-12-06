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
#include "MP3GPIO.hpp"
#include "labSPI.hpp"
#include "ADCDrv.hpp"

/*
    This Drv should do the following
    -Setup and maintain the SPI communication with the 
*/
class VS1011Drv
{
    public:
        static VS1011Drv * instance();
        uint16_t * m_transBuf;

        //This is the queue handle that the flash read Task will fill
        QueueHandle_t m_transmitQueue;

        //should set up the SPI and init the VS1011
        //Start the tasks for sending data to the decoder
        //set queue handler
        void initialize();

        //this function is purely for testing purposes, it sends a series of commands that initiate the sine test in the decoder
        //a sine wave will be output as audio, will simply test that the decoder is working.
        void SineTestStart();

        void SineTestStop();

        //#######################################TASKS#############################################
        //this task will wait on the queue being filled by the flash reader
        //it will take the first item off the queue and send it to the decoder
        void SendToDecoder();

        void volUp();

        void volDown();

        //This is used for initial setup, sending config commands to the flash
        void SendSCIWriteCommand(uint8_t regAddr, uint16_t writeData);

        uint16_t SendSCIReadCommand(uint8_t regAddr);
        
        //sets the member variable queue handler to the one created by the 
        //task
        void setQueueHandler(QueueHandle_t &flashQueue);

        void selectDCS();

        void deselectDCS();

        void selectxDCS();

        void deselectxDCS();

    private:
        static VS1011Drv * m_instance;
        LabSPI * m_SPIDrv;

        ADCDrv m_ADCDrv;
        //chip select GPIO pins
        //one for SCI commands, the other for SDI data
        MP3GPIO * xDCS;
        MP3GPIO * DCS;
        //this is the decoder output that lets us know when we can send data.
        MP3GPIO * DREQ;
        MP3GPIO * RST;
        VS1011Drv();
        ~VS1011Drv();
};