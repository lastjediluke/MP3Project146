#include "VS1011Drv.hpp"

VS1011Drv * VS1011Drv::instance()
{
    if(!m_instance)
    {
        m_instance = VS1011Drv();
    }
    return m_instance;
}

//should set up the SPI and init the VS1011
//Start the tasks for sending data to the decoder
//set queue handler
void VS1011Drv::initialize(QueueHandle_t &flashQueue, TaskHandle_t &taskHandle)
{
    m_SPIDrv = new MP3SPI();
    //initialize the SPI
    m_SPIDrv.initializeDecoderSPI();
    setQueueHandler(flashQueue);

    DREQ = new MP3GPIO(wat, wat);
    DCS = new MP3GPIO(pinNumDCS, portNumDCS);
    xDCS = new MP3GPIO(pinNumxDCS, portNumxDCS);
    DREQ.setAsInput();
    DCS.setAsOutput();
    xDCS.setAsOutput();

    /*
    TODO
        Fill in the correct port and pin numbers for DCS, xDCS, and DREG
        SETUP the initialization commands to be sent to the decoder.
    */
    uint8_t const SCI_MODE_REG = 0x00;
    uint8_t const SCI_CLOCKF_REG = 0x03;
    uint8_t const SCI_VOL_REG = 0x0B;

    sendSCIcommand(SCI_MODE_REG, 0x04); //send reset
    sendSCIcommand(SCI_CLOCKF_REG, 0x97F4);   //send 38,900 =  0x8000 + (12.34 MHz / 2000)    some nonsense in data sheet, should enable internal clock doubling
    sendSCIcommand(SCI_VOL_REG, 0x0404);  //set to volume slightly under max, left and right channel put at -2.0 db, so send (4*256) + 4 = 1028 (data sheet black magic calculations) 

    xTaskCreate(SendToDecoder, "vSendToDecoder", 1024, NULL, 1, &taskHandle);

}

//this is driven by a command handler. you can use the terminal to execute the start and stop of the test
void VS1011Drv::SineTestStart()
{
    uint8_t const test = 0b00001111;    //test freq of 5000Hz

    deselectxDCS();
    selectDCS();

    m_SPIDrv.transferMP3(0x53);
    m_SPIDrv.transferMP3(0xEF);
    m_SPIDrv.transferMP3(0x6E);
    m_SPIDrv.transferMP3(test);
    m_SPIDrv.transferMP3(0x00);
    m_SPIDrv.transferMP3(0x00);
    m_SPIDrv.transferMP3(0x00);
    m_SPIDrv.transferMP3(0x00);

    deselectDCS();
}

void VS1011Drv::SineTestStop()
{
    deselectxDCS();
    selectDCS();

    m_SPIDrv.transferMP3(0x45);
    m_SPIDrv.transferMP3(0x78);
    m_SPIDrv.transferMP3(0x69);
    m_SPIDrv.transferMP3(0x74);
    m_SPIDrv.transferMP3(0x00);
    m_SPIDrv.transferMP3(0x00);
    m_SPIDrv.transferMP3(0x00);
    m_SPIDrv.transferMP3(0x00);

    deselectDCS();    
}

//this task will wait on the queue being filled by the flash reader
//it will take the first item off the queue and send it to the decoder
void VS1011Drv::SendToDecoder()
{
    uint8_t * buf_ptr;
    while(1)
    {
        if(xQueueReceive(m_transmitQueue, m_transBuf, portMAX_DELAY))
        {
            buf_ptr = m_transBuf;
            /*
                we grab 512 bytes off of the queue
                we can only safely send 32 bytes at a time though
                When DREQ is high, it means we can send at least 32 bytes to the decoder (FIFO has room for 32 bytes)
                We need to ensure we send off all 512 bytes of the current buffer before we grab the next

                TODO:
                    -make sure we stop sending data when the user requests a "pause"
                    -if we switch songs, we need to empty the buffer and queue
                    -NEW_SONG AND PAUSED ARE PLACEHOLDERS FOR STATES DETERMINED BY USER INPUT ELSEWHERE 
            */
            while(!(buf_ptr > &m_transBuf[511]))    //while the buf_ptr has not exceeded the end of the chunk
            {
                if(NEW_SONG)
                {
                    break;
                }   
                if(DREQ.getLevel() && !PAUSED) //can we send 32 bytes?
                {
                    deselectxDCS();
                    selectDCS(); 
                    //send 32 bytes
                    for(int i =0; i<32; i++)
                    {
                        m_SPIDrv.transferMP3(*buf_ptr);
                        buf_ptr++;
                    }
                    deselectDCS();    
                }
            }
        }
    }
}

//This is used for initial setup, sending config commands to the flash
void VS1011Drv::SendSCIWriteCommand(unit8_t regAddr, uint16_t writeData)
{
    uint8_t const SCI_WRITE = 0x02;
    deselectDCS();
    selectxDCS();

    m_SPIDrv.transferMP3(SCI_WRITE);    //send write command
    m_SPIDrv.transferMP3(regAddr);
    m_SPIDrv.transferMP3(writeData & 0xFF00);  //send upper half
    m_SPIDrv.transferMP3(writeData & 0x00FF);    //send lowerhalf

    deselectxDCS();
    while(!DREQ.getLevel());
}

void VS1011Drv::setVolume(uint16_t db)
{
    uint16_t newdB = (db/0.5) * 256 + (db/0.5);

    SendSCIWriteCommand(0x0B, newdB);
    while(!DREQ.getLevel());
}
//sets the member variable queue handler to the one created by the 
//task
void VS1011Drv::setQueueHandler(QueueHandle_t &flashQueue)
{
    m_transmitQueue = flashQueue;
}

void VS1011Drv::selectDCS()
{
    DCS.setLow();
}

void VS1011Drv::deselectDCS()
{
    DCS.setHigh();
}

void VS1011Drv::selectxDCS()
{
    xDCS.setLow();
}

void VS1011Drv::deselectxDCS()
{
    xDCS.setHigh();
}