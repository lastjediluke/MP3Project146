#include "VS1011Drv.hpp"
#include "Interrupt.h"

Interrupt intr;

VS1011Drv::VS1011Drv()
{
    volume = 0;
}
VS1011Drv * VS1011Drv::m_instance = NULL;

VS1011Drv * VS1011Drv::instance()
{
    if(!m_instance)
    {
        m_instance = new VS1011Drv();
        m_instance->initialize();
    }
    return m_instance;
}

void eint3Handler(void){
	intr.HandleInterrupt();
}

//should set up the SPI and init the VS1011
//Start the tasks for sending data to the decoder
//set queue handler
void VS1011Drv::initialize()
{
    //initialize the SPI
    m_SPIDrv = new LabSPI();
    m_SPIDrv->initializeDecoderSPI();

    //init ADC for volume control
    m_ADCDrv.AdcInitBurstMode();
    m_ADCDrv.AdcSelectPin(ADCDrv::k1_31);

    RST = new MP3GPIO(19, 1);
    DREQ = new MP3GPIO(28, 1);
    DCS = new MP3GPIO(20, 1);
    xDCS = new MP3GPIO(23, 1);
    DREQ->setAsInput();
    DCS->setAsOutput();
    xDCS->setAsOutput();
    RST->setAsOutput();

    //HW RESET
    RST->setLow();
    vTaskDelay(100);
    RST->setHigh();
    while(!DREQ->getLevel());

    DCS->setHigh();
    xDCS->setHigh(); 
    while(!DREQ->getLevel());

    //SW RESET
    SendSCIWriteCommand(0x00, 0x0800 | 0x0004);
    while(!DREQ->getLevel());
    /*
    TODO
        Fill in the correct port and pin numbers for DCS, xDCS, and DREG
        SETUP the initialization commands to be sent to the decoder.
    */
    uint8_t const SCI_MODE_REG = 0x0;
    uint8_t const SCI_CLOCKF_REG = 0x3;
    uint8_t const SCI_VOL_REG = 0xB;
    uint8_t const SCI_AUDATA_REG = 0x5;
    uint16_t result = SendSCIReadCommand(0x00);
    SendSCIWriteCommand(SCI_MODE_REG, 0x0800); //SendSCIWriteCommand(SCI_MODE_REG, 0x0800 | 0x04); //send reset
    
    printf("SCI_MODE REG: %d \n",result);
    SendSCIWriteCommand(SCI_AUDATA_REG, 0xAC45);    //44100Hz stereo
    SendSCIWriteCommand(SCI_CLOCKF_REG, 0x2000);   //CLKI internal clock is now 24.3 MHz
    SendSCIWriteCommand(SCI_VOL_REG, 0x0404);  //set to volume slightly under max, left and right channel put at -2.0 db, so send (4*256) + 4 = 1028 (data sheet black magic calculations) 
    // calculation to set it to -2.0db: 0x04 * 0.5 = -2.0db
    

    result = SendSCIReadCommand(0xB);
    printf("VOL REG: %d \n",result);

    result = SendSCIReadCommand(0x5);
    printf("AUData: %d \n",result);

    //xTaskCreate(SendToDecoder, "vSendToDecoder", 1024, NULL, 1, &taskHandle);
    //xTaskCreate(monitorVolume, "vMonitorVolume", 1024, NULL, 1, NULL);

    // create interrupt buttons for volume up and down
    intr.Initialize();
    // when interrupt is triggered, go to Eint3Handler()
    isr_register(EINT3_IRQn, eint3Handler);
    //specify port, pin, IsrPointer, and interruptCondition for interrupt buttons
    intr.AttachInterruptHandler(2, 0, volUp, kRisingEdge);
    intr.AttachInterruptHandler(0, 0, volDown, kRisingEdge);
}

//this is driven by a command handler. you can use the terminal to execute the start and stop of the test
void VS1011Drv::SineTestStart()
{
    uint8_t const test = 0b00011111;    //test freq of 5000Hz
    uint8_t const SCI_MODE_REG = 0x0;
    uint8_t const SCI_CLOCKF_REG = 0x3;
    uint8_t const SCI_VOL_REG = 0xB;
    uint8_t const SCI_AUDATA_REG = 0x5;
    //HW reset
    RST->setLow();
    vTaskDelay(100);
    RST->setHigh();
    while(!DREQ->getLevel());

    //Send test commands
    SendSCIWriteCommand(SCI_AUDATA_REG, 0xAC45);    //44100Hz stereo
    SendSCIWriteCommand(SCI_CLOCKF_REG, 0x2000);   //CLKI internal clock is now 24.3 MHz
    SendSCIWriteCommand(SCI_VOL_REG, 0x0404); 
    SendSCIWriteCommand(SCI_MODE_REG, 0x0800 | 0x0020); //enable SDI tests
    uint16_t result = SendSCIReadCommand(0x00);
    printf("SCI_MODE REG: %d",result);
    deselectxDCS();

    selectDCS();
    m_SPIDrv->transferMP3(0x53);
    m_SPIDrv->transferMP3(0xEF);
    m_SPIDrv->transferMP3(0x6E);
    m_SPIDrv->transferMP3(test);
    m_SPIDrv->transferMP3(0x00);
    m_SPIDrv->transferMP3(0x00);
    m_SPIDrv->transferMP3(0x00);
    m_SPIDrv->transferMP3(0x00);
    deselectDCS();
    while(!DREQ->getLevel());
}

void VS1011Drv::SineTestStop()
{
    selectDCS();
    deselectxDCS();

    m_SPIDrv->transferMP3(0x45);
    m_SPIDrv->transferMP3(0x78);
    m_SPIDrv->transferMP3(0x69);
    m_SPIDrv->transferMP3(0x74);
    m_SPIDrv->transferMP3(0x00);
    m_SPIDrv->transferMP3(0x00);
    m_SPIDrv->transferMP3(0x00);
    m_SPIDrv->transferMP3(0x00);

    deselectDCS();
    while(!DREQ->getLevel());    
}

//this task will wait on the queue being filled by the flash reader
//it will take the first item off the queue and send it to the decoder
// void VS1011Drv::SendToDecoder()
// {
//     uint8_t * buf_ptr;
//     while(1)
//     {
//         if(xQueueReceive(m_transmitQueue, m_transBuf, portMAX_DELAY))
//         {
//             buf_ptr = m_transBuf;
//             /*
//                 we grab 512 bytes off of the queue
//                 we can only safely send 32 bytes at a time though
//                 When DREQ is high, it means we can send at least 32 bytes to the decoder (FIFO has room for 32 bytes)
//                 We need to ensure we send off all 512 bytes of the current buffer before we grab the next

//                 TODO:
//                     -make sure we stop sending data when the user requests a "pause"
//                     -if we switch songs, we need to empty the buffer and queue
//                     -NEW_SONG AND PAUSED ARE PLACEHOLDERS FOR STATES DETERMINED BY USER INPUT ELSEWHERE 
//             */
//             while(!(buf_ptr > &m_transBuf[511]))    //while the buf_ptr has not exceeded the end of the chunk
//             {
//                 if(NEW_SONG)
//                 {
//                     break;
//                 }   
//                 if(DREQ.getLevel() && !PAUSED) //can we send 32 bytes?
//                 {
//                     deselectxDCS();
//                     selectDCS(); 
//                     //send 32 bytes
//                     for(int i =0; i<32; i++)
//                     {
//                         m_SPIDrv->transferMP3(*buf_ptr);
//                         buf_ptr++;
//                     }
//                     deselectDCS();    
//                 }
//             }
//         }
//     }
// }

// actual volume ranges from 0 to 254dB. If volume reaches 255dB, it triggers powerdown mode
void volUp(uint16_t incr = 0x3232)
{
    uint16_t result = SendSCIReadCommand(0xB);
    incr = ((incr / 0.5) << 8) + (incr / 0.5);
    result -= incr;
    SendSCIWriteCommand(0xB, result);
}

void volDown(uint16_t decr = 0x3232) 
{
    uint16_t result = SendSCIReadCommand(0xB);
    incr = ((incr / 0.5) << 8) + (incr / 0.5);
    result += incr;
    SendSCIWriteCommand(0xB, result);
}

//This is used for initial setup, sending config commands to the flash
void VS1011Drv::SendSCIWriteCommand(uint8_t regAddr, uint16_t writeData)
{
    uint8_t const SCI_WRITE = 0x02;
    uint8_t highByte = (writeData >> 8);
    uint8_t lowByte = writeData;
    deselectDCS();
    selectxDCS();

    m_SPIDrv->transferMP3(SCI_WRITE);    //send write command
    m_SPIDrv->transferMP3(regAddr);
    m_SPIDrv->transferMP3(highByte);
    m_SPIDrv->transferMP3(lowByte);

    while(!DREQ->getLevel());
    deselectxDCS();
}

uint16_t VS1011Drv::SendSCIReadCommand(uint8_t regAddr)
{
    deselectDCS();
    selectxDCS();

    m_SPIDrv->transfer(0x03);
    m_SPIDrv->transfer(regAddr);
    uint8_t high = m_SPIDrv->transfer(0xa0);
    uint8_t low = m_SPIDrv->transfer(0xf4);

    while(!DREQ->getLevel());
    deselectxDCS();

    uint16_t result = (high << 8) | low;
    return result;
}

//sets the member variable queue handler to the one created by the 
//task
void VS1011Drv::setQueueHandler(QueueHandle_t &flashQueue)
{
    m_transmitQueue = flashQueue;
}

void VS1011Drv::selectDCS()
{
    DCS->setLow();
}

void VS1011Drv::deselectDCS()
{
    DCS->setHigh();
}

void VS1011Drv::selectxDCS()
{
    xDCS->setLow();
}

void VS1011Drv::deselectxDCS()
{
    xDCS->setHigh();
}