#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "hibernation.h"

void initHibernationHW()
{

    SYSCTL_RCGCHIB_R |= SYSCTL_RCGCHIB_R0;
    _delay_cycles(3);

    HIB_IM_R |= HIB_IM_WC;
    HIB_CTL_R |= HIB_CTL_CLK32EN;
    while(!(HIB_MIS_R & HIB_IM_WC));
    HIB_CTL_R |= HIB_CTL_RTCEN;
}

int32_t getCurrentSeconds()
{
    int32_t seconds = (HIB_RTCC_R%86400);
    return seconds;
}
