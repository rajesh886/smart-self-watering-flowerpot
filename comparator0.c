/**
 * CREATED BY RAJESH RAYAMAJHI
 */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "comparator0.h"
#include "wait.h"

#define DEINT_PIN  (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 5*4)))

#define COMPARATOR_ZERO_MASK 128
#define DEINT_MASK 32

uint32_t volume = 0;

void initCOMP()
{

    // Enable clocks
    SYSCTL_RCGCACMP_R  |=  SYSCTL_RCGCACMP_R0;                 // Analog Comparator Run Mode Clock Gating Control
    SYSCTL_RCGCGPIO_R  |=  SYSCTL_RCGCGPIO_R2;                 // turning on clock for Port C for CO-
    _delay_cycles(3);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    GPIO_PORTC_DIR_R   &= ~COMPARATOR_ZERO_MASK ;              // enable input on PC7 pin
    GPIO_PORTC_DR2R_R  |=  COMPARATOR_ZERO_MASK ;              // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTC_DEN_R   &= ~COMPARATOR_ZERO_MASK ;              // disabling digital function for port C
    GPIO_PORTC_AMSEL_R |=  COMPARATOR_ZERO_MASK ;
    GPIO_PORTC_AFSEL_R |=  COMPARATOR_ZERO_MASK ;              // use peripheral to drive CO-

    COMP_ACREFCTL_R |=  0x0000020F ;                           // Configure the internal voltage reference to 2.469 V ( Analog Comparator Reference Voltage Control )
    COMP_ACCTL0_R   |=  0x0000040C ;                           // Configure the comparator to use the internal voltage reference (Analog Comparator Control 0)


}


void initDEINT()
{

    SYSCTL_RCGCGPIO_R  |=  SYSCTL_RCGCGPIO_R4;    //port E
    _delay_cycles(3);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    GPIO_PORTE_DIR_R  |= DEINT_MASK;
    GPIO_PORTE_DR2R_R |= DEINT_MASK;
    GPIO_PORTE_DEN_R  |= DEINT_MASK;


}

void initTIMER1()
{

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    _delay_cycles(3);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Configure Timer 1 as the time base
    TIMER1_CTL_R   &= ~TIMER_CTL_TAEN;                                              // turn-off counter before reconfiguring
    TIMER1_CFG_R    = TIMER_CFG_32_BIT_TIMER;;                                                            // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R   = TIMER_TAMR_TACMR | TIMER_TAMR_TAMR_CAP | TIMER_TAMR_TACDIR; // configure for edge time mode, count up
    TIMER1_CTL_R    = 0;
    TIMER1_TAV_R    = 0;                                    // zero counter for first period

}


uint32_t getVolume()
{
    DEINT_PIN = 1;

    waitMicrosecond(10000);

    //while(!(COMP_ACSTAT0_R & COMP_ACSTAT0_OVAL));

    TIMER1_TAV_R = 0;                                    // zero counter for first period

    DEINT_PIN = 0;
    TIMER1_CTL_R |= TIMER_CTL_TAEN ;                    //enable timer

    while(COMP_ACSTAT0_R & COMP_ACSTAT0_OVAL);

    volume = TIMER1_TAV_R;

    volume = ((volume * 25)/100)-80.25;  //80.25 is the capacitance value when the water bottle is empty.
                                         //the ratios of time to capacitance is 100.
    volume = (volume / 0.47);            //the ratios of capacitance to volume of water is 0.47

    return volume;

}


