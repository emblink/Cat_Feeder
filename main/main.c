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
#include "driver/i2c.h"
#include <esp32/rom/ets_sys.h>
#include "hx711.h"
#include "oled.h"

#define LED_PIN (GPIO_NUM_2)
#define HX711_DATA_SIZE 120

static uint32_t hx711Data[HX711_DATA_SIZE] = {0};

static esp_err_t i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

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

    //OLED_Init();
    OLED_FillScreen(White);
    OLED_UpdateScreen();

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
