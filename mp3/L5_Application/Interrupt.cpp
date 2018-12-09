#include "Interrupt.h"
#include <stddef.h>
#include "LPC17xx.h"
#include "uart0_min.h"

Interrupt::Interrupt(){
	readOut = 0;
}

void Interrupt::Initialize(){
	NVIC_EnableIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(ADC_IRQn);
}

bool Interrupt::AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr, InterruptCondition condition){
	if (port == 1 || port > 2 || port < 0) return false;
	if (pin > 0 || pin > 31) return false;
	if (pin_isr == NULL) return false;

	//set lookup
	pin_isr_map[port][pin] = pin_isr;
	
	//enable interrutps
	if (port == 0){
		if (condition == kRisingEdge) {
			LPC_GPIOINT->IO0IntEnR |= (1 << pin);
		}
		else if (condition == kFallingEdge) {
				LPC_GPIOINT->IO0IntEnF |= (1 << pin);
		}
		else {
			LPC_GPIOINT->IO0IntEnR |= (1 << pin);
			LPC_GPIOINT->IO0IntEnF |= (1 << pin);
		}
	}
	else if (port == 2) {
		if (condition == kRisingEdge) {
			LPC_GPIOINT->IO2IntEnR |= (1 << pin);
		}
		else if (condition == kFallingEdge) {
			LPC_GPIOINT->IO2IntEnF |= (1 << pin);
		} 
		else {
			LPC_GPIOINT->IO2IntEnR |= (1 << pin);
			LPC_GPIOINT->IO2IntEnF |= (1 << pin);
		}
	}
	else
		return false;
		
	return true;
}

bool Interrupt::AttachInterruptHandlerADC(uint8_t port, uint32_t pin, IsrPointer pin_isr, PinNum pinNum) {
	if (port == 1 || port > 2 || port < 0) return false;
	if (pin > 0 || pin > 31) return false;
	if (pin_isr == NULL) return false;

	pin_isr_map[3][pin] = pin_isr;

	bool match = false;

	switch(pinNum)
	{
		case k0_25:
			LPC_ADC->ADINTEN |= (1 << 2);
			match = true;
			break;
		case k0_26:
			LPC_ADC->ADINTEN |= (1 << 3);
			match = true;
			break;
		case k1_30:
			LPC_ADC->ADINTEN |= (1 << 4);
			match = true;
			break;
		case k1_31:
			LPC_ADC->ADINTEN |= (1 << 5);
			match = true;
			break;
	}

	if (!match)	return false;

	return true;
}

void Interrupt::HandleInterrupt() {
	if (LPC_GPIOINT->IO0IntStatR) {
		LPC_GPIOINT->IO0IntClr |= LPC_GPIOINT->IO0IntStatR;
		pin_isr_map[0][LPC_GPIOINT->IO0IntStatR]();
	}
	else if (LPC_GPIOINT->IO0IntStatF) {
		LPC_GPIOINT->IO0IntClr |= LPC_GPIOINT->IO0IntStatF;
		pin_isr_map[0][LPC_GPIOINT->IO0IntStatF]();
	}
	else if (LPC_GPIOINT->IO2IntStatR) {
		LPC_GPIOINT->IO2IntClr |= LPC_GPIOINT->IO2IntStatR;
		pin_isr_map[1][LPC_GPIOINT->IO2IntStatR]();
	}
	else {
		LPC_GPIOINT->IO2IntClr |= LPC_GPIOINT->IO2IntStatF;
		pin_isr_map[1][LPC_GPIOINT->IO2IntStatF]();
	}
}

void Interrupt::HandleADCInterrupt() {
	uart0_puts ("inside function\n");
	if (LPC_ADC->ADSTAT == 2) {
		readOut = (LPC_ADC->ADDR2 >> 4) & (0xfff);
		uart0_puts ("2\n");
	}
	else if (LPC_ADC->ADSTAT == (1 << 3)) {
		readOut = (LPC_ADC->ADDR3 >> 4) & (0xfff);
		uart0_puts ("3\n");
	}
	else if (LPC_ADC->ADSTAT == (1 << 4)) {
		readOut = (LPC_ADC->ADDR4 >> 4) & (0xfff);
		uart0_puts ("4\n");
	}
	else if (LPC_ADC->ADSTAT == (1 << 5)) {
		readOut = (LPC_ADC->ADDR5 >> 4) & (0xfff);
		uart0_puts ("5\n");
	}

	uart0_puts ("end of function\n");
	pin_isr_map[3][26]();
}

float Interrupt::GetReadOut() {
	return readOut;
}