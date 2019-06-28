#include "hx711.h"
#include "stdint.h"

void hx711Init(void)
{

}

bool hx711GetStatus(void)
{
    return true;
}

bool hx711ReadChannel(Hx711Channel channel, uint32_t *data)
{
    return true;
}

/* When PD_SCK pin changes from low to high
and stays at high for longer than 60µs, HX711
enters power down mode (Fig.3). */
void hx711PowerDown(void)
{

}

/* After a reset or power-down event, input
selection is default to Channel A with a gain of 128. */
void hx711Resume(void)
{

}

