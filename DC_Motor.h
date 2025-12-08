#ifndef __DC_MOTOR
#define __DC_MOTOR

typedef enum {
    DIRECTION_CW = 0,
    DIRECTION_CCW
} tenuDirection;

void DC_Init();
void DC_Start(unsigned char MotorID, tenuDirection Direction, unsigned char Speed);
void DC_Stop(unsigned char MotorID);

#endif