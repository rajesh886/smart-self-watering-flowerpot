//  motor controller code
//  Port used is D and pin is PD3

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"

#define MOTOR_PIN  (*((volatile uint32_t *)(0x42000000 + (0x400073FC-0x40000000)*32 + 3*4)))

#define MOTOR_MASK 8
/**
 * main.c
 */

void initMOTORHW()
{
    SYSCTL_RCGCGPIO_R  |=  SYSCTL_RCGCGPIO_R3;    //port D
    _delay_cycles(3);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    GPIO_PORTD_DIR_R  |= MOTOR_MASK;
    GPIO_PORTD_DR2R_R |= MOTOR_MASK;
    GPIO_PORTD_DEN_R  |= MOTOR_MASK;


}

void enablePump()
{
    MOTOR_PIN = 1;
}

void disablePump()
{
    MOTOR_PIN = 0;
}
