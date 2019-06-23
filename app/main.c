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

#define LED_PIN (GPIO_NUM_2)
#define EXTERNAL_LED_PIN (GPIO_NUM_18)

static uint32_t blink = 0;

static void ledBlinkTask(void* arg)
{
    printf("Configure gpio pin\n");

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_pulldown_dis(LED_PIN);
    gpio_pullup_dis(LED_PIN);
    gpio_intr_disable(LED_PIN);

    gpio_config_t config;
    config.pin_bit_mask = GPIO_SEL_18;
    config.intr_type = GPIO_INTR_DISABLE;
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&config);

    printf("Gpio pin Configured\n");


    for (;;) {
        blink ^= 0x01;
        gpio_set_level(LED_PIN, blink);
        gpio_set_level(EXTERNAL_LED_PIN, blink ^ 0x01);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}


void app_main()
{
    printf("Hello world!\n");

    //start led blink task
    xTaskCreate(ledBlinkTask, "ledBlinkTask", 2048, NULL, 10, NULL);
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
        vTaskDelay(1200 / portTICK_PERIOD_MS);
    }
}
