#ifndef __DIO__
#define __DIO__

#define DIO_DIR_OUTPUT          1
#define DIO_DIR_INPUT           0
#define DIO_STATE_HIGH          1 
#define DIO_STATE_LOW           0

#define DDR_D          (volatile uint8_t *)(0x2A)
#define PORT_D          (volatile uint8_t *)(0x2B)

inline void Dio_SetRegisterBit(volatile unsigned char* Register, unsigned char Bit);

inline void Dio_ResetRegisterBit(volatile unsigned char* Register, unsigned char Bit);

inline void Dio_ToggleRegisterBit(volatile unsigned char* Register, unsigned char Bit);

void Dio_SetPinDirection(volatile unsigned char* Register, unsigned char Pin, unsigned char Direction);

void Dio_SetPinState(volatile unsigned char* Register, unsigned char Pin, unsigned char State);

unsigned char Dio_GetPinState(volatile unsigned char* Register, unsigned char Pin);
void reverse_string(char* str, int length);
void int_to_string(int n, char* str);
int get_string_length(int val);

#endif /*__DIO__*/
