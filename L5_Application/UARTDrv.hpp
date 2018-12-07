#include <stdio.h>
#include "io.hpp"
#include "LPC17xx.h"
#include <stddef.h>
#include "sys_config.h"
#include "uart0_min.h"
class UARTDrv
{
    public:
        UARTDrv();

        bool initializeUART(uint8_t pUART, void (*ISR_handler)(void));

        void uart_transmit(char tr);

        char uart_receive();
    private:
        uint8_t m_UARTp;
};