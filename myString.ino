/******************************************************************
file: String Functions Code
author: Nour Eldien, Tarek Mahmoud Younes, Kareem Magdy
brief: includes definitions string manipulation functions
*******************************************************************/
/*************Include Section Start**********/
#include "myString.h"

/*************Include Section End**********/


/*************Defined Functions Section Start**********/

// Function definition to convert a string of digits to a 16-bit unsigned integer.
uint16_t simple_atoi(char* str)
{
    uint16_t res = 0;      // Initialize the result variable to 0 (unsigned 16-bit integer).
    for (int i = 0; str[i] != '\0'; ++i) // Loop through the characters of the string until the null terminator ('\0') is found.
    {
        if (str[i] >= '0' && str[i] <= '9') // Check if the current character is a digit ('0' through '9').
        {
            // Accumulate the result: 
            // 1. Multiply the current result by 10 (shifting the existing digits one decimal place left).
            // 2. Add the integer value of the current digit (str[i] - '0' converts ASCII digit to int value).

            res = res * 10 + str[i] - '0';
        }
    }
    return res;  // Return the final converted integer value.
}

// Function definition to reverse a string in place.
void reverse_string(char* str, int length) 
{
    int start = 0;  // Initialize the starting index to the beginning of the string.
    int end = length - 1;  // Initialize the ending index to the last character of the string.
    char temp;  // Declare a temporary character variable for swapping.
    while (start < end)  // Loop while the start index is less than the end index.
    {
        temp = str[start]; // Store the character at the start index.
        str[start] = str[end]; // Move the character from the end to the start position.
        str[end] = temp;  // Place the stored start character into the end position.
        start++; // Increment the start index (move towards the middle).            
        end--;  // Decrement the end index (move towards the middle).
    }
}   

// Function definition to convert an integer to its string representation.
void int_to_string(int n, char* str) 
{
    int i = 0;  // Index to keep track of the current position in the result string 'str'.
    int is_negative = 0;  // Flag to indicate if the original number was negative.

    if (n == 0) // Handle the special case where the input number is 0.
    {
        str[i++] = '0';  //Place the character '0' in the string and increment index.
        str[i] = '\0';  // Null-terminate the string.
        return; // Exit the function.
    }

    if (n < 0)  // Handle negative numbers.
    {
        is_negative = 1;  is_negative = 1;   // Set the negative flag.
        n *= -1;   // Convert the number to positive for digit extraction.
    }

    while (n != 0) // Extract digits from the number in reverse order (least significant first).
    {
        int remainder = n % 10; // Get the last digit (remainder).
        str[i++] = remainder + '0'; // Convert the digit value to its ASCII character ('0' + remainder) and store it, then increment index.
        n = n / 10; // Remove the last digit (integer division).
    }

    if (is_negative) // If the original number was negative, append the minus sign.
    {
        str[i++] = '-'; // Place the '-' character and increment index.
    }

    str[i] = '\0';  // Null-terminate the constructed string.
    reverse_string(str, i);  // The digits and sign are currently in reverse order, so call reverse_string to correct them.
}

// Function definition to calculate the length of a null-terminated string.
int simple_strlen(char* str) 
{
    int len = 0; // Initialize length counter.
    while (str[len] != '\0') // Loop until the null terminator ('\0') is encountered.
    {
        len++;  // Increment the length counter.
    }
    return len;  // Return the final length.
}

// Function definition for linear value mapping/scaling.
long map(long x, long in_min, long in_max, long out_min, long out_max) 
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;    // Apply the scaling formula
}

// Function definition to count the number of digits in a positive integer.
int number_of_digits(int num)
{
    int cnt = 1;  // Initialize the counter to 1 (this is an unusual starting point/implementation).
    while(num > 0) // Loop continues as long as the number isn higher than zero.
    {
        num /= 10;  // Divide the number by 10, effectively removing the last digit.
        cnt += 1;  // Increment the counter. (This results in a return value of digits + 1).
    }
    return cnt;  // Return the final count.
}


/*************Defined Functions Section End**********/