#include "UARTDrv.hpp"

UARTDrv::UARTDrv()
{

}

bool UARTDrv::initializeUART(uint8_t UARTp, void (*ISR_handler)(void))
{
    if(UARTp != 2 && UARTp != 3)
    {
        return false;
    }
    m_UARTp = UARTp;
    //Set bits in PCONP register... may need to know which UART 0/2/3 
    //in PCKSEL1 select UART 2/3

    //Set baud rate in 2/3LCR, DLAB = 1

    //IN 0/2/3FCR enable FIFO
 
    //SELECT UART PINS through the PINSEL and PINMODES: NOT HAVE PULLDOWN RESISTORS
    const uint32_t baud = 38400;
    const uint8_t dll = sys_get_cpu_clock() / (16 * baud);
    switch(UARTp)
    {
        case 2:
            LPC_SC->PCONP |= (1 << 24);
            LPC_SC->PCLKSEL1 |= (1 << 16);

            LPC_UART2->LCR |= (1 << 7);
            LPC_UART2->DLL = dll;
            LPC_UART2->LCR = 3;
            LPC_UART2->LCR &= ~(1 << 7);

            LPC_UART2->FCR |= (1 << 0);
            LPC_UART2->FCR |=  (1 << 1) | (1 << 2);

            //LPC_UART2->IER |= (1 << 0);
            //LPC_UART2->IER |= (1 << 2);
            NVIC_EnableIRQ(UART2_IRQn);
            isr_register(UART2_IRQn, ISR_handler);

            LPC_PINCON->PINSEL4 |= (2 << 16);   //TX2
            LPC_PINCON->PINSEL4 |= (2 << 18);   //TX2
            break;
        case 3:
            LPC_SC->PCONP |= (1 << 25);
            LPC_SC->PCLKSEL1 |= (1 << 18);

            LPC_UART3->LCR |= (1 << 7);
            LPC_UART3->DLL = dll;
            LPC_UART3->LCR = 3;
            LPC_UART3->LCR &= ~(1 << 7);

            LPC_UART3->FCR |= (1 << 0);
            LPC_UART3->FCR |=  (1 << 1) | (1 << 2);

            //LPC_UART3->IER |= (1 << 0);
            //LPC_UART3->IER |= (1 << 2);
            NVIC_EnableIRQ(UART3_IRQn);
            isr_register(UART3_IRQn, ISR_handler);
            
            LPC_PINCON->PINSEL9 |= (3 << 24);   //TX3
            LPC_PINCON->PINSEL9 |= (3 << 26);   //RX3
            break;
    }
    //Enable UART INT's, set DLAB = 0 in U2/3LCR,

    //Enable interrupt in NVIC with ISE register
}

void UARTDrv::uart_transmit(char tr)
{
    if(m_UARTp == 2)
    {
        while(!(LPC_UART2->LSR & (1 << 5)));
        LPC_UART2->THR = tr;
    }
    else if(m_UARTp == 3)
    {
        while(!(LPC_UART3->LSR & (1 << 5)));
        LPC_UART3->THR = tr;
    }
    //printf("t%c ",tr);
}

char UARTDrv::uart_receive()
{
    char ret;
    if(m_UARTp == 2)
    {
        while(!(LPC_UART2->LSR & (1 << 0)));
        ret = LPC_UART2->RBR;
    }
    else if(m_UARTp == 3)
    {
        while(!(LPC_UART3->LSR & (1 << 0)));
        ret = LPC_UART3->RBR;
    }
    //uart0_puts("r");
    return ret;
}