#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "hx711.h"

Hx711Handle *handle = NULL;
static bool inited = false;

bool hx711Init(Hx711Handle *hndl)
{
    inited = false;
    if (hndl != NULL) {
        if (hndl->readCb && hndl->writeCb && hndl->delayCb) {
            handle = hndl;
            inited = true;
            for (uint32_t k = 0; k < 5; k++) {
                for (uint32_t i = 0; i < 24; i++) {
                    handle->writeCb(handle->sclkPin, true);
                    handle->delayCb(1);
                    handle->writeCb(handle->sclkPin, false);
                    handle->delayCb(1);
                }
            }
        }
    }
    return inited;
}

bool hx711GetStatus(void)
{
    return handle->readCb(handle->dataPin) == false;
}

bool hx711ReadChannel(Hx711Channel channel, uint32_t *data)
{
    if (!inited || channel < Hx711ChannelA128 || channel > Hx711ChannelA64 || data == NULL)
        return false;

    *data = 0;
    for (uint32_t i = 0; i < 24; i++) {
    	handle->writeCb(handle->sclkPin, true);
        handle->delayCb(1);
        if (handle->readCb(handle->dataPin)) {
            (*data)++;
        }
        *data <<= 1;
        handle->writeCb(handle->sclkPin, false);
        handle->delayCb(1);
    }
    *data &= 0xFFFFFF;

    for (uint32_t i = 0; i < channel - 24; i++) {
        handle->writeCb(handle->sclkPin, true);
        handle->delayCb(1);
        handle->writeCb(handle->sclkPin, false);
    }

    return *data > 0;
}

/* When PD_SCK pin changes from low to high
and stays at high for longer than 60µs, HX711
enters power down mode (Fig.3). */
void hx711PowerDown(void)
{
    handle->writeCb(handle->sclkPin, true);
    handle->delayCb(60);
}

/* After a reset or power-down event, input
selection is default to Channel A with a gain of 128. */
void hx711PowerUp(void)
{
    handle->writeCb(handle->sclkPin, false);
    handle->delayCb(10);
}

