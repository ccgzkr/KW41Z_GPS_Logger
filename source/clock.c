#include "clock.h"

void clockSettings(void)
{
    const sim_clock_config_t simConfig;
    simConfig->er32kSrc = 0U;
    simConfig->clkdiv1 = 0x00010000U;

    CLOCK_SetSimConfig(&simConfig);

    CLOCK_SetTpmClock(1U);
}

