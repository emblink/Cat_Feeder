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

typedef bool (*pinReadCallback)(uint32_t pin);
typedef void (*pinWriteCallback)(uint32_t pin, bool state);
typedef uint32_t (*delayUsCb)(uint32_t us);

typedef struct Hx711Handle {
    uint32_t dataPin;
    uint32_t sclkPin;
    pinReadCallback readCb;
    pinWriteCallback writeCb;
    delayUsCb delayCb;
} Hx711Handle;

bool hx711Init(Hx711Handle *handle);
bool hx711GetStatus(void);
bool hx711ReadChannel(Hx711Channel channel, uint32_t *data);
void hx711PowerDown(void);
void hx711PowerUp(void);

#endif // __HX_711_H