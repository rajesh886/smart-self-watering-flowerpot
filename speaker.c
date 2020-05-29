//  Speaker code
//  Port is A and pin is PA7

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "wait.h"

#define SPEAKER_PIN  (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 7*4)))

#define SPEAKER_MASK 128
/**
 * main.c
 */

void initSPEAKERHW()
{

    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;    //Timer 2
    SYSCTL_RCGCGPIO_R  |=  SYSCTL_RCGCGPIO_R0 |SYSCTL_RCGCGPIO_R5;    //port A
    _delay_cycles(3);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    GPIO_PORTA_DIR_R  |= SPEAKER_MASK;
    GPIO_PORTA_DR2R_R |= SPEAKER_MASK;
    GPIO_PORTA_DEN_R  |= SPEAKER_MASK;

    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER2_TAILR_R = 40000000;
    TIMER2_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
    NVIC_EN0_R |= 1 << (INT_TIMER2A-16);             // turn-on interrupt 39 (TIMER2A
}

void timer2Isr()
{

    SPEAKER_PIN ^=1;
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
}

void playBatteryLowAlert()
{
    uint16_t periods[2];
    periods[0]=13514;
    periods[1]=36077;
    uint8_t size = sizeof(periods)/sizeof(int);
    uint16_t count = 0;

    while(count!=size){
            TIMER2_TAILR_R = periods[0];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(210625);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;

            TIMER2_TAILR_R = periods[1];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(473025);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;

            TIMER2_TAILR_R = periods[0];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(210625);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;

            TIMER2_TAILR_R = periods[1];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(473025);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;

            count++;
    }

}

void playWaterLowAlert()
{
    uint16_t period[2];
    period[0]=15168;
    period[1]=40495;
    uint8_t size = sizeof(period)/sizeof(int);
    uint16_t count = 0;

    while(count!=size){
            TIMER2_TAILR_R = period[0];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(47401);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
            TIMER2_TAILR_R = period[1];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(177168);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
            TIMER2_TAILR_R = period[0];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(47401);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;
            TIMER2_TAILR_R = period[1];
            TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
            waitMicrosecond(177168);
            TIMER2_CTL_R &= ~TIMER_CTL_TAEN;

            count++;
    }

}
