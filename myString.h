#ifndef MYSTRING_H
#define MYSTRING_H

#include <stdint.h>

uint16_t simple_atoi(char* str);
void reverse_string(char* str, int length);
void int_to_string(int n, char* str);
int simple_strlen(char* str);

#endif