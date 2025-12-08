#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <avr/io.h>

typedef enum {
    BTN_PRESSED,
    BTN_RELEASED
} ButtonState;

typedef struct sButton {
    volatile uint8_t* ddr;
    volatile uint8_t* port;
    volatile uint8_t* pin;
    uint8_t pinNumber;
    ButtonState (*getState)(struct sButton* self);
} Button;

void Button_init(Button* self, volatile uint8_t* ddr, volatile uint8_t* port, volatile uint8_t* pin, uint8_t pinNumber);

#endif