/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <esp32/rom/ets_sys.h>
#include "hx711.h"

#define LED_PIN (GPIO_NUM_2)
#define HX711_DATA_SIZE 120

static uint32_t hx711Data[HX711_DATA_SIZE] = {0};

static void ledBlinkTask(void *arg)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_pulldown_dis(LED_PIN);
    gpio_pullup_dis(LED_PIN);
    gpio_intr_disable(LED_PIN);
    for (;;) {
        gpio_set_level(LED_PIN, true);
        vTaskDelay(5);
        gpio_set_level(LED_PIN, false);
        vTaskDelay(995);
    }
}

static void weightTask(void *arg)
{
    #define HX711_DATA_PIN GPIO_NUM_34
    #define HX711_SCLK_PIN GPIO_NUM_27

    gpio_set_direction(HX711_DATA_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_dis(HX711_DATA_PIN);
    gpio_pullup_dis(HX711_DATA_PIN);
    gpio_intr_disable(HX711_DATA_PIN);

    gpio_set_direction(HX711_SCLK_PIN, GPIO_MODE_OUTPUT);
    gpio_pulldown_dis(HX711_SCLK_PIN);
    gpio_pullup_dis(HX711_SCLK_PIN);
    gpio_intr_disable(HX711_SCLK_PIN);

    static Hx711Handle handle = {
        .dataPin = HX711_DATA_PIN,
        .sclkPin = HX711_SCLK_PIN,
        .readCb = gpio_get_level,
        .writeCb = gpio_set_level,
        .delayCb = ets_delay_us,
    };

    hx711Init(&handle);

    for (;;) {
        static uint32_t index = 0;
        if (hx711GetStatus() == Hx711StatusReady) {
            if (hx711ReadChannel(Hx711ChannelA64, &hx711Data[index]) == Hx711StatusOk)
                index++;
        }
        if (index >= HX711_DATA_SIZE) {
            uint64_t mean = 0;
            for (uint32_t i = 0; i < HX711_DATA_SIZE; i++)
                mean += hx711Data[i];
            mean /= index;
            index = 0;
            printf("HX711 data == %lli\n", mean);
        }
        vTaskDelay(12);
    }
}

void app_main()
{
    printf("Started!\n");
    xTaskCreate(ledBlinkTask, "ledBlinkTask", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(weightTask, "weightTask", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    while(1) 
    {
        vTaskDelay(1000);
    }
}
