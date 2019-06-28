#ifndef __HX_711_H
#define __HX_711_H

#include "stdint.h"
#include "stdbool.h"

typedef enum Hx711Channel{
    Hx711ChannelA128 = 25,
    Hx711ChannelB32 = 26,
    Hx711ChannelA64 = 27,
    Hx711ChannelCount,
} Hx711Channel;

//typedef void (*Hx711WriteCallback) (uint32_t data, uint32_t len);
void hx711Init(void /*set spi callback */);
bool hx711GetStatus(void);
bool hx711ReadChannel(Hx711Channel channel, uint32_t *data);
void hx711PowerDown(void);
void hx711Resume(void);

#endif // __HX_711_H