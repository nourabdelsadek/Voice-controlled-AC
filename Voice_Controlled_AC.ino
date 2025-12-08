
#include "Adc.h"
#include "Dio.h"
#include "DC_Motor.h"
#include "Lcd.h"
#include "myString.h"
#include "Pwm.h"
#include "Button.h"
#include "Uart.h"
#include <stdint.h>

#define outTempPin 0
#define inTempPin 5
#define buttonPin 4
Button btnSetTemp;
char receivedTarget[3]="25";
int targetTemp=25;
int lastIn;
int lastOut;
int lastTarget;
char target[3]="25";
int state = 0;
char motorState;


void hood(int speed){
  DC_Start(0, DIRECTION_CCW, speed);
  LCD_String_xy(1,0,"Backward");
  motorState='B';
}

void fan(int speed){
  DC_Start(0, DIRECTION_CW, speed);
  LCD_String_xy(1,0,"Forward ");
  motorState='F';
}

void stop(){
  DC_Stop(0);
  LCD_String_xy(1,0,"Stopped");
  motorState='S';
}

int main(){

  Uart_Init();
  LCD_Init();
  Button_init(&btnSetTemp, &DDRB, &PORTB, &PINB, PB4);
  Adc_Init();
  PWM_Init();
  DC_Init();
  LCD_String_xy(0,0, "Target:");
  LCD_String_xy(0,10, "In:");
  LCD_String_xy(1,10, "Out:");
  while(1)
  {

    char difference;
    int outTemp;
    int inTemp;
    char out[3];
    char in[3];
    char sp[4];
    char all[15];

    if (Uart_IsDataAvailable())
    {
      Uart_ReadString(receivedTarget, 3);
      targetTemp=simple_atoi(receivedTarget);
    }
    if (btnSetTemp.getState(&btnSetTemp) == BTN_PRESSED && state == 0)
    {
      Uart_SendString("Start\n",6);
      state = 1;
    }
    else if (btnSetTemp.getState(&btnSetTemp) == BTN_PRESSED && state == 1)
    {
      Uart_SendString("Stop\n",5);
      state = 0;
    }
    while(btnSetTemp.getState(&btnSetTemp) == BTN_PRESSED);// Debounce/wait for the button to be released.
    //if uart available store data in target
    //if button pressed start recording message
    //if button released stop recording

    inTemp = Adc_ReadChannel(inTempPin)*19/44;
    outTemp = Adc_ReadChannel(outTempPin)*19/44;
    int_to_string(inTemp,in);
    int_to_string(outTemp,out);
    int_to_string(targetTemp,target);
    if (number_of_digits(inTemp)<number_of_digits(lastIn) || number_of_digits(outTemp)<number_of_digits(lastOut) || number_of_digits(targetTemp)<number_of_digits(lastTarget))
    {
      LCD_Clear();
      LCD_String_xy(0,0, "Target:");
      LCD_String_xy(0,10, "In:");
      LCD_String_xy(1,10, "Out:");
    }
    LCD_String_xy(0,7,target);
    LCD_String_xy(0,13,in);
    LCD_String_xy(1,14,out);
    //Uart_SendString(target,2);

    difference = abs(inTemp-targetTemp);
    int speed = min(map(difference, 0, 20, 165, 255), 255);
    if(difference > 2){
      if(inTemp > targetTemp){
        if(outTemp < inTemp)
          fan(speed);
        else
          hood(speed);
      }
      else if(outTemp > inTemp)
        fan(speed);
      else{
        stop();
      }
    }
    else 
      stop();

    int cnt = 3;
    while(cnt--){
      sp[cnt] = speed%10 + '0';
      speed /= 10;
    }
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
  lastIn=inTemp;
  lastOut=outTemp;
  lastTarget=targetTemp;
  _delay_ms(100);
  }

}
  
