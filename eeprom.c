
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "wait.h"


uint32_t eeprominit()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    SYSCTL_GPIOHBCTL_R = 0;

    SYSCTL_RCGCEEPROM_R |= SYSCTL_RCGCEEPROM_R0;
    _delay_cycles(6);

    while(EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
    uint32_t status;
    status = EEPROM_EESUPP_R;
    if(status & (EEPROM_EESUPP_PRETRY | EEPROM_EESUPP_ERETRY))
    {
        return 2;
    }
    SYSCTL_SREEPROM_R &= ~SYSCTL_SREEPROM_R0;
    _delay_cycles(6);
    status = EEPROM_EESUPP_R;
    if(status & (EEPROM_EESUPP_PRETRY | EEPROM_EESUPP_ERETRY))
    {
        return 2;
    }

    return 0;
}

uint32_t readeeprom(uint32_t block, uint32_t offset)
{
    while(EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
    EEPROM_EEBLOCK_R  = block;
    EEPROM_EEOFFSET_R  = offset;
    while(EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
    return EEPROM_EERDWR_R;
}

void writeeeprom(uint32_t block, uint32_t offset, uint32_t data)
{
    while(EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
    EEPROM_EEBLOCK_R  = block;
    EEPROM_EEOFFSET_R  = offset;
    while(EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
    EEPROM_EERDWR_R = data;
}

void eraseeeprom(uint32_t block, uint32_t offset)
{
    while(EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
    EEPROM_EEBLOCK_R  = block;
    EEPROM_EEOFFSET_R  = offset;
    while(EEPROM_EEDONE_R & EEPROM_EEDONE_WORKING);
    EEPROM_EERDWR_R = 0xFFFFFFFF;
}



