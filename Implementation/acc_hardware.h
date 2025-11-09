#ifndef ACC_HARDWARE_H
#define ACC_HARDWARE_H

#include <stdint.h>

// Hardware Abstraction Layer Function Declarations

float Read_Distance_Sensor(void);
float Read_Speed_Sensor(void);
void Apply_Throttle_Brake(float dM);
void Hardware_Timer_ClearFlag(void);
void Hardware_Timer_Enable(void);
void Hardware_Timer_Disable(void);
void Hardware_Init(void);
void LCD_Display_Distance(float distance);
void LCD_Display_Speed(float speed);
void LCD_Display_ACC_Status(uint8_t status);

#endif // ACC_HARDWARE_H

