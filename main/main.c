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
#include "hx711.h"

#define LED_PIN (GPIO_NUM_2)
#define VSPI_MOSI (GPIO_NUM_23)
#define VSPI_MISO (GPIO_NUM_19)
#define VSPI_SCLK (GPIO_NUM_18)

static spi_device_handle_t spi;

static const spi_bus_config_t spiConfig = {
    .mosi_io_num = VSPI_MOSI,
    .miso_io_num = VSPI_MISO,
    .sclk_io_num = VSPI_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 0,
    .flags = 0,
    .intr_flags = 0
};

static spi_device_interface_config_t devcfg = {
    .clock_speed_hz = 1*1000*1000, // Tmin = 0,2us = 5Mhz, set to 1Mhz
    .mode = 0,                     // SPI mode 0
    .spics_io_num = -1,            // no CS pin
    .queue_size = 2,
    .flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA,
};


static void ledBlinkTask(void *arg)
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

static void weightTask(void *arg)
{
    hx711Init();
    static uint32_t value = 0;
    for (;;) {
        hx711ReadChannel(Hx711ChannelA128, &value);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    printf("Started!\n");

    printf("Spi Init\n");
    esp_err_t err = ESP_OK;
    err |= spi_bus_initialize(VSPI_HOST, &spiConfig, 0);
    printf("spi_bus_initialize Passed!\n");
    err |= spi_bus_add_device(VSPI_HOST, &devcfg, &spi);
    printf("spi_bus_add_device Passed!\n");
    ESP_ERROR_CHECK(err);
    printf("Spi Init Success!\n");

    //start led blink task
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

    while(1) {
        printf("main task delay...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
