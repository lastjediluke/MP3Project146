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
        IsrPointer pin_isr_map[2][32];
    public:
        Interrupt();
        void Initialize();
        bool AttachInterruptHandler(uint8_t port, uint32_t pin, IsrPointer pin_isr, InterruptCondition condition);
        void HandleInterrupt();
};
#endif
