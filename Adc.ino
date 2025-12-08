/******************************************************************
file: UART Functions Code
author: Nour Eldin, Tarek Mahmoud Younes, Kareem Magdy
brief: includes definitions of ADC functions
*******************************************************************/

/*************Include Section Start**********/

#if !defined(__AVR_ATmega328P__) // Conditional include for device-specific I/O registers if the target is not implicitly defined as ATmega328P.
#include <avr/iom328p.h>
#endif
#include "Adc.h"

/*************Include Section End**********/

/*************Defined Functions Section Start**********/
void Adc_Init(void) //ADC initialization function
{
    // AREF = AVcc
    ADMUX = (1<<REFS0);
  
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

unsigned short Adc_ReadChannel(unsigned char ch) //Function for ADC reading from a certain channel/pin.
{

  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch = ch & 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0b11111000)|ch; // clears the bottom 3 bits before ORing
 
  // start single convertion
  // write ’1′ to ADSC
  ADCSRA = ADCSRA | (1<<ADSC);
 
  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));
 
  return (ADC);

}

/*************Defined Functions Section End**********/