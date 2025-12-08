#ifndef __LCD__
#define __LCD__ 

void LCD_Command(unsigned char cmnd);
void LCD_Char (unsigned char char_data);
void LCD_Init (void);
void LCD_String (char *str);
void LCD_String_xy (char row, char pos, char *str);
void LCD_Clear();
void LCD_CreateCustomChar(unsigned char location, unsigned char *data_bytes);

#endif /*__LCD__*/