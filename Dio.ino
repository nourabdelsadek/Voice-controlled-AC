/******************************************************************
file: DIO Functions Code
author: Nour Eldien, Tarek Mahmoud Younes, Kareem Magdy
brief: includes definitions digital I/O functions
*******************************************************************/

/*************Include Section Start**********/

#include "Dio.h" // Includes the custom header file for DIO function prototypes.

/*************Include Section End**********/

/*************Defined Functions Section Start**********/

// Note: 'inline' suggests the compiler should attempt to replace the function call with the function body to optimize speed.

// Function to set a specific bit within a volatile hardware register.
inline void Dio_SetRegisterBit(volatile unsigned char* Register, unsigned char Bit)
{
  *Register = *Register | (1 << Bit);
}

// Function to clear a specific bit within a volatile hardware register.
inline void Dio_ResetRegisterBit(volatile unsigned char* Register, unsigned char Bit)
{
  *Register = *Register & ~(1 << Bit);
}

// Function to toggle the state of a specific bit within a volatile hardware register.
inline void Dio_ToggleRegisterBit(volatile unsigned char* Register, unsigned char Bit)
{
  *Register = *Register ^ (1 << Bit);
}

// Function to read the current state of a specific bit within a volatile hardware register.
inline unsigned char Dio_GetRegisterBit(volatile unsigned char* Register, unsigned char Bit)
{
  return (*Register >> Bit) & (unsigned char)0x01;
}

// Function to set the direction (INPUT/OUTPUT) of a specific microcontroller pin.
void Dio_SetPinDirection(volatile unsigned char* Register, unsigned char Pin, unsigned char Direction) 
{
    if (Direction == 0)
    {
      Dio_ResetRegisterBit(Register, Pin);
    } else{
      Dio_SetRegisterBit(Register, Pin);
    }
}

// Function to set the logic state (LOW/HIGH) of a specific microcontroller pin configured as output.
void Dio_SetPinState(volatile unsigned char* Register, unsigned char Pin, unsigned char State) 
{
    if (State == 0)
    {
      Dio_ResetRegisterBit(Register, Pin);
    } else{
      Dio_SetRegisterBit(Register, Pin);
    }
}

// Function to read the current logic state of a specific microcontroller pin.
unsigned char Dio_GetPinState(volatile unsigned char* Register, unsigned char Pin) 
{
  return Dio_GetRegisterBit(Register, Pin);
}

/*************Defined Functions Section End**********/