/******************************************************************
file: Button Functions Code
author: Nour Eldien, Tarek Mahmoud Younes, Kareem Magdy
brief: includes definitions of button handling functions
*******************************************************************/

/*************Include Section Start**********/

#include "Button.h"  // Include the custom header file, likely defining the 'Button' structure and 'ButtonState' enum.

/*************Include Section End**********/

/*************Defined Functions Section Start**********/

// Static function (local to this file) that reads the current physical state of the button pin.
static ButtonState getButtonState(struct sButton* self) 
{
    if ( (*self->pin & (1 << self->pinNumber)) ) 
    {
        return BTN_PRESSED; // If the pin bit is SET (HIGH), return BTN_PRESSED.
    } else {
        return BTN_RELEASED; // If the pin bit is CLEAR (LOW), return BTN_RELEASED.
    }
}

// Function to initialize the Button structure and configure the associated I/O pin.
void Button_init(Button* self, volatile uint8_t* ddr, volatile uint8_t* port, volatile uint8_t* pin, uint8_t pinNumber) 
{
    // Store pointers to the pin's I/O registers
    self->ddr = ddr;
    self->port = port;
    self->pin = pin;
    // Store the specific pin number
    self->pinNumber = pinNumber;

    // --- Pin Configuration ---

    
   
    *self->ddr &= ~(1 << self->pinNumber); // Set the pin direction as INPUT: Clear the corresponding bit in the DDRx register
    *self->port &= ~(1 << self->pinNumber);// Disable the internal Pull-Up Resistor


    self->getState = getButtonState; // Assign the static state-checking function to the function pointer within the structure.
}

/*************Defined Functions Section End**********/