#pragma once
#ifndef INTERRUPT_H
#define INTERRUPT_H
#include <stdint.h>

enum InterruptCondition{
	kRisingEdge,
	kFallingEdge,
	kBothEdges
};

typedef void (*IsrPointer)(void);

class Interrupt {
    private:
        IsrPointer pin_isr_map[3][32];
        float readOut;
    public:
        enum PinNum
        {
            k0_25,       // AD0.2 <-- Light Sensor -->
            k0_26,       // AD0.3
            k1_30,       // AD0.4
            k1_31,       // AD0.5
        };

        Interrupt();
        void Initialize();
        bool AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr, InterruptCondition condition);
        bool AttachInterruptHandlerADC(uint8_t port, uint32_t pin, IsrPointer pin_isr, PinNum pinNum);
        void HandleInterrupt();
        void HandleADCInterrupt();
        float GetReadOut();
};
#endif
