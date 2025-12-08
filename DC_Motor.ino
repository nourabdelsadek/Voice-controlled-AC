#include "DC_Motor.h"
#include "Pwm.h"
#include <util/delay.h>
#if !defined(__AVR_ATmega328P__)
#include <avr/iom328p.h>
#endif

// Define CPU Frequency (Crucial for _delay_ms to work correctly)
// Assuming a standard 16MHz Arduino/AVR clock speed.
#define F_CPU 16000000UL 

// Pin assignments for motor (MotorID 0: PD2 and PD3)
unsigned char MotorArray[][2] = {
    {2, 3} 
};

// --- PWM Functions (Timer 1, OC1B/PB2/Digital Pin 10) ---



// --- DC Motor Control Functions (Port D) ---

void DC_Init(){
    // Dynamically set all motor pins as output pins
    for (unsigned char i = 0; i < sizeof(MotorArray)/sizeof(MotorArray[0]); ++i) {
        DDRD |= (1 << MotorArray[i][0]);
        DDRD |= (1 << MotorArray[i][1]);
    }
}

void DC_Start(unsigned char MotorID, tenuDirection Direction, unsigned char Speed){

    if (Direction == DIRECTION_CW)
    {
        // Set Pin 1 HIGH and Pin 2 LOW for CW rotation
        PORTD |= (1 << MotorArray[MotorID][0]);
        PORTD &= ~(1 << MotorArray[MotorID][1]);
    } else{
        // Set Pin 1 LOW and Pin 2 HIGH for CCW rotation
        PORTD &= ~(1 << MotorArray[MotorID][0]);
        PORTD |= (1 << MotorArray[MotorID][1]);
    }
    
    // Apply the PWM speed to the motor's Enable pin (via OCR1B/PB2)
    Pwm_SetDutyCycle(Speed);

}

void DC_Stop(unsigned char MotorID){
    // Active Brake: Sets both pins HIGH, shorting the motor leads to VCC/Ground
    PORTD |= (1 << MotorArray[MotorID][0]);
    PORTD |= (1 << MotorArray[MotorID][1]);
    
    // Also stop the PWM signal
    Pwm_SetDutyCycle(0);
}