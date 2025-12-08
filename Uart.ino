/******************************************************************
file: UART Functions Code
author: Nour Eldien, Tarek Mahmoud Younes, Kareem Magdy
brief: includes definitions of the only used UART functions in the project 
*******************************************************************/
/*************Include Section Start**********/

#if !defined(__AVR_ATmega328P__)
#include <avr/iom328p.h>
#endif
#include "Uart.h"
#include <avr/io.h>
/*************Include Section End**********/

/*************Definition Section Start**********/
#define F_CPU 16000000UL  // Define the microcontroller's clock frequency as 16 MHz (Unsigned Long)
#define _BV(bit) (1 << (bit))  // Macro to set a specific bit position (e.g., _BV(TXEN0) is (1 << TXEN0)).
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))  // Macro to check if a specific bit in a Special Function Register (sfr) is set.
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))  // Macro to check if a specific bit in an SFR is clear.
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit)) // Busy-wait loop: continues as long as the specified bit in the SFR is CLEAR (low).
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))  // Busy-wait loop: continues as long as the specified bit in the SFR is SET (high).
/*************Definition Section End**********/

/*************Defined Functions Section Start**********/

// Function to initialize the UART peripheral.
void Uart_Init(void) {
    Uart_SetBaudRate(9600);  // Set the initial baud rate to 9600.
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);  // Enable Transmitter (TXEN0) and Receiver (RXEN0).
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // Set character size to 8-bit data 
}

// Function to set UART baud rate dynamically
void Uart_SetBaudRate(unsigned short BuadRate)  
{
    unsigned short UBBR = ( (F_CPU / 16) /  BuadRate ) - 1; // Calculate baud rate register value
    UBRR0L = (UBBR & 0xFF); // Load lower 8 bits of UBRR value
    UBRR0H = (( UBBR >> 8 ) & 0xFF); // Load higher 8 bits of UBRR value
}

// Function to transmit a single byte of data.
void Uart_SendChar(unsigned char DataByte) 
{
    loop_until_bit_is_set(UCSR0A, UDRE0);  // Wait until the Data Register Empty (UDRE0) flag in UCSR0A is set (high).
    UDR0 = DataByte;  // Write the data byte to the UDR0 register, starting transmission.
}

// Function to check if received data is available in the buffer.
unsigned char Uart_IsDataAvailable(void) {
    return (UCSR0A & (1 << RXC0)); // Return the state of the Receiver Complete (RXC0) flag in UCSR0A.
}

// Function to read a single byte of received data.
unsigned char Uart_ReadData(void) {
    loop_until_bit_is_set(UCSR0A, RXC0); // Wait until the Receive Complete (RXC0) flag in UCSR0A is set (high).
    return UDR0; // Return the received data byte from the UDR0 register.
}

// Function to read a string of a specified size.
void Uart_ReadString(char DataString[], unsigned char Size){
    int i; // Loop counter.
    for (i=0; i<Size; i++) // Loop 'Size' times to read 'Size' characters.
    {
        DataString[i] = Uart_ReadData();  // Read one character at a time and store it in the array.
    }
}

// Function to read a number (as a sequence of digit characters) and convert it to an unsigned char.
unsigned char Uart_ReadNumber(){
    int i; // Loop counter
    unsigned char Number = 0; // Initialize the numerical result to 0.
    while ( (UCSR0A & ( 1 << RXC0)) ) // Loop as long as the Receive Complete (RXC0) flag is set (data is available).
    {
        unsigned char ReadData = Uart_ReadData(); // Read the character.

        if(ReadData < '0' && ReadData > '9')  // Check if the character is NOT a digit (less than '0' or greater than '9').
        {
            continue; // Skip non-digit characters
        }
        // Convert the digit character to its integer value (ReadData - '0').
        // Accumulate the number: multiply the current number by 10 and add the new digit.
        Number = Number*10 + (ReadData - '0'); 
    }
    return Number;  // Return the final converted number.
}

// Function to send a string of a specified size.
void Uart_SendString(char DataString[], unsigned char Size){
    int i; // Loop counter.
    for (i=0; i<Size; i++)  // Loop 'Size' times to send 'Size' characters.
    {
        Uart_SendChar(DataString[i]); // Send one character at a time.
    }
}

/*************Defined Functions Section End**********/