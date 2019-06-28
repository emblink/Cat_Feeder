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
#include "hx711.h"

#define LED_PIN (GPIO_NUM_2)

static void ledBlinkTask(void* arg)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_pulldown_dis(LED_PIN);
    gpio_pullup_dis(LED_PIN);
    gpio_intr_disable(LED_PIN);

    static uint32_t blink = 0;
    for (;;) {
        blink ^= 0x01;
        printf("Blink!\n");
        gpio_set_level(LED_PIN, blink);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    printf("Started!\n");

    //start led blink task
    xTaskCreate(ledBlinkTask, "ledBlinkTask", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
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

    while(1) {
        printf("main task delay...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
