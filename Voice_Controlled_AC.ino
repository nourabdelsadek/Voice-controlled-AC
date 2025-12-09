
#include "Adc.h"  // Analog-to-Digital Converter driver
#include "Dio.h"  // Digital Input/Output driver
#include "DC_Motor.h" // DC Motor control driver
#include "Lcd.h"  // LCD display driver
#include "myString.h" // Custom string manipulation functions (e.g., int_to_string, simple_atoi)
#include "Pwm.h"  // Pulse Width Modulation driver
#include "Button.h" // Button driver
#include "Uart.h" // Universal Asynchronous Receiver/Transmitter (Serial) driver
#include <stdint.h> // Standard integer types
#include <util/delay.h> // Header for _delay_ms function (added based on its usage)

#define outTempPin 0  // ADC Channel for the outdoor temperature sensor
#define inTempPin 5 // ADC Channel for the indoor temperature sensor
#define buttonPin 4 // Pin for the control button (PB4 based on init)
Button btnSetTemp;  // Structure/object to manage the control button state
char receivedTarget[3]="25";  // Buffer to store the 2-digit target temperature received via UART
int targetTemp=25;  // Integer version of the target temperature
int lastIn; // Stores the previous indoor temperature for LCD clear check
int lastOut;  // Stores the previous outdoor temperature for LCD clear check
int lastTarget; // Stores the previous target temperature for LCD clear check
char target[3]="25";  // String version of the target temperature for LCD display
int state = 0;  // System state: 0 (Off), 1 (Running)
char motorState; // Motor state indicator: 'B' (Backward/Hood), 'F' (Forward/Fan), 'S' (Stopped)

/**
 *Controls the DC Motor to act as a Hood (e.g., exhausting hot air).
 *speed The PWM duty cycle/speed (0-255).
 */
void hood(int speed){
  DC_Start(0, DIRECTION_CCW, speed);  // Start DC Motor 0 in Counter-Clockwise (CCW) direction
  LCD_String_xy(1,0,"Backward");  // Display motor direction on LCD
  motorState='B'; // Update motor state
}

/**
 *Controls the DC Motor to act as a Fan (e.g., pulling in cool air or circulating).
 *speed The PWM duty cycle/speed (0-255).
 */
void fan(int speed){
  DC_Start(0, DIRECTION_CW, speed); // Start DC Motor 0 in Clockwise (CW) direction
  LCD_String_xy(1,0,"Forward ");  // Display motor direction on LCD
  motorState='F'; // Update motor state
}

/**
 *Stops the DC Motor.
 */
void stop(){
  DC_Stop(0); // Stop DC Motor 0
  LCD_String_xy(1,0,"Stopped"); // Display motor state on LCD
  motorState='S'; // Update motor state
}

// --- Initialization Section ---
int main(){

  Uart_Init();  // Initialize UART communication
  LCD_Init(); // Initialize LCD display
  Button_init(&btnSetTemp, &DDRB, &PORTB, &PINB, PB4);  // Initialize button on PB4
  Adc_Init(); // Initialize ADC for temperature sensors
  PWM_Init(); // Initialize PWM for motor speed control
  DC_Init();  // Initialize DC Motor driver
  // Initial LCD display setup
  LCD_String_xy(0,0, "Target:");
  LCD_String_xy(0,10, "In:");
  LCD_String_xy(1,10, "Out:");

  // --- Main Control Loop ---
  while(1)
  {

    char difference;  // Stores the absolute difference between indoor and target temp
    int outTemp;  // Stores the calculated outdoor temperature
    int inTemp; // Stores the calculated indoor temperature
    char out[3];  // String buffer for outdoor temp (2 digits + null term)
    char in[3]; // String buffer for indoor temp
    char sp[4]; // String buffer for motor speed (3 digits + null term)
    char all[15]; // Combined string buffer for UART transmission

    // 1. Voice/UART Target Temperature Update
    if (Uart_IsDataAvailable())
    {
      Uart_ReadString(receivedTarget, 3); // Read up to 2 characters (plus null) from UART
      targetTemp=simple_atoi(receivedTarget); // Convert received string to integer target temperature
    }
    // 2. System State (Start/Stop) Control via Button
    if (btnSetTemp.getState(&btnSetTemp) == BTN_PRESSED && state == 0)  
    {
      Uart_SendString("Start\n",6); // Send "Start" confirmation via UART
      state = 1;  // Change state to Running
    }
    else if (btnSetTemp.getState(&btnSetTemp) == BTN_PRESSED && state == 1)
    {
      Uart_SendString("Stop\n",5);  // Change state to Running (Automatic)
      state = 0;  // Change state to Manual/Off
    }
    while(btnSetTemp.getState(&btnSetTemp) == BTN_PRESSED);// Debounce/wait for the button to be released.

    // 3. Read and Convert Temperatures
    // The formula (*19/44) is used to calibratre the temperature sensor
    inTemp = Adc_ReadChannel(inTempPin)*19/44;
    outTemp = Adc_ReadChannel(outTempPin)*19/44;

    // Convert integer temperatures to strings for LCD display
    int_to_string(inTemp,in);
    int_to_string(outTemp,out);
    int_to_string(targetTemp,target);

    // 4. LCD Clear Check
    // If any temperature/target number of digits changes (e.g., 9 -> 10 or 10 -> 9), clear LCD to prevent old digits from remaining.
    if (number_of_digits(inTemp)<number_of_digits(lastIn) || number_of_digits(outTemp)<number_of_digits(lastOut) || number_of_digits(targetTemp)<number_of_digits(lastTarget))
    {
      LCD_Clear();
      LCD_String_xy(0,0, "Target:");
      LCD_String_xy(0,10, "In:");
      LCD_String_xy(1,10, "Out:");
    }

    // 5. Display Current Temperatures and Target on LCD
    LCD_String_xy(0,7,target);  // Display Target Temp
    LCD_String_xy(0,13,in); // Display Indoor Temp
    LCD_String_xy(1,14,out);  // Display Target Temp
    //Uart_SendString(target,2);

    difference = abs(inTemp-targetTemp);  // Calculate absolute temperature error

    // Map the temperature difference (0-20) to a PWM speed (165-255).
    // min(..., 255) ensures speed does not exceed max PWM duty cycle.
    int speed = min(map(difference, 0, 20, 165, 255), 255);
    if(difference > 2){ // Only run motor if difference is greater than 2 degrees
      if(inTemp > targetTemp){  // Indoor temp is too high (needs cooling)
        if(outTemp < inTemp)  // If outdoor air is cooler than indoor air
          fan(speed); // Pull in cooler outdoor air (Fan)
        else
          hood(speed);  // Otherwise, exhaust indoor hot air (Hood)
      }
      else if(outTemp > inTemp) // If outdoor temp is warmer than indoor, pull it in.
        fan(speed);
      else{
        stop();
      }
    }
    else 
      stop(); 
// Convert speed integer to string (sp) by repeatedly taking modulo 10
    int cnt = 3;
    while(cnt--){
      sp[cnt] = speed%10 + '0';
      speed /= 10;
    }
// Construct the comma-separated data string (all)
// Format: MotorState,TargetTemp,InTemp,OutTemp,Speed\n (15 characters total)
  for (int i=0; i<(sizeof(all)/sizeof(all[0]))-1;i++)
  {
    if (i==0)
      all[0]=motorState;

    else if (i == 1 || i == 4 || i == 7 || i == 10)
      all[i]=',';

    else if(i <= 3)
      all[i]=receivedTarget[i-2];

    else if(i <= 6)
      all[i]=in[i-5];

    else if(i <= 9)
      all[i]=out[i-8];
    else
      all[i]=sp[i-11];
  }
  all[14] = '\n';
  Uart_SendString(all,15);  
// 8. Update Last Values and Delay
  lastIn=inTemp;
  lastOut=outTemp;
  lastTarget=targetTemp;
  _delay_ms(100);
  }

}
  
