#include  "Pwm.h"
void PWM_Init() {
    // 1. Set Digital Pin 10 (PB2) as an output pin
    DDRB |= (1 << DDB2); 

    // 2. Configure Timer 1 for Fast PWM Mode 14 (ICR1 as TOP)
    TCCR1A |= (1 << WGM11); 
    TCCR1B |= (1 << WGM13) | (1 << WGM12); 

    // 3. Set Non-Inverting Mode for OC1B (COM1B1)
    TCCR1A |= (1 << COM1B1);

    // 4. Set TOP value for 8-bit resolution (0-255)
    ICR1 = 255; 

    // 5. Start Timer 1 with No Prescaler (CS10)
    TCCR1B |= (1 << CS10);
    
    // 6. Initialize PWM to 0%
    Pwm_SetDutyCycle(0);
}

void Pwm_SetDutyCycle(int duty) {
    OCR1B = duty; 
}