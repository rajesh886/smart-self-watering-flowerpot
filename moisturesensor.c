
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD/Temperature Sensor
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz
// Stack:           4096 bytes (needed for sprintf)

// Hardware configuration:
// AIN1/PE2 is driven by the sensor

// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "wait.h"
#include "uart0.h"
#include "adc0.h"

// PortE masks
#define AIN1_MASK 4

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initmoisturesensor()
{

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    // Note UART on port E must use APB
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable clocks
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
    _delay_cycles(3);

    // Configure AIN1 as an analog input
    GPIO_PORTE_AFSEL_R |= AIN1_MASK;                 // select alternative functions for AN1 (PE2)
    GPIO_PORTE_DEN_R &= ~AIN1_MASK;                  // turn off digital operation on pin PE2
    GPIO_PORTE_AMSEL_R |= AIN1_MASK;                 // turn on analog operation on pin PE2
}

float getMoisturePercentage(){

    uint16_t raw;

    float moisturepercent = 0;

    initmoisturesensor();

    initAdc0Ss3();

    // Use AIN2 input with N=64 hardware sampling
    setAdc0Ss3Mux(1);
    setAdc0Ss3Log2AverageCount(6);

    raw = readAdc0Ss3();
    moisturepercent = ((raw+0.5) / 4096.0 * 3.3);
    moisturepercent = (moisturepercent/2.8)*100;

    return moisturepercent;
}

