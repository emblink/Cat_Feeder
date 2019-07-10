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
#include "ssd1306.h"

#define LED_PIN (GPIO_NUM_2)
#define HX711_MEASURMENTS_COUNT 10

#define I2C_MASTER_FREQ_HZ 400000U
#define I2C_PORT_NUM I2C_NUM_0

static esp_err_t i2c_master_init()
{   
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_21;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = GPIO_NUM_22;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_PORT_NUM, &conf);
    return i2c_driver_install(I2C_PORT_NUM, conf.mode, 0, 0, 0);
}

// static esp_err_t i2c_master_write_slave(uint8_t address, const uint8_t *data_wr, size_t size)
// {
//     #define ACK_CHECK_EN 0x1 /*!< I2C master will check ack from slave*/
//     i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//     i2c_master_start(cmd);
//     i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
//     i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
//     i2c_master_stop(cmd);
//     esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000);
//     i2c_cmd_link_delete(cmd);
//     return ret == ESP_OK;
// }

static void ledBlinkTask(void *arg)
{
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_pulldown_dis(LED_PIN);
    gpio_pullup_dis(LED_PIN);
    gpio_intr_disable(LED_PIN);
    for (;;) 
    {
        gpio_set_level(LED_PIN, true);
        vTaskDelay(5);
        gpio_set_level(LED_PIN, false);
        vTaskDelay(995);
    }
}

static int compare(const void * a, const void * b)
{
   return (*(int32_t *)a - *(int32_t *)b);
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
    static uint32_t tare = 0;
    static bool calibration = true;
    static int32_t measurments[HX711_MEASURMENTS_COUNT];
    printf("Calibration!\n");
    SSD1306_GotoXY(0, 0);
    SSD1306_Puts("Calibration! ", &Font_7x10, SSD1306_COLOR_WHITE);
    SSD1306_UpdateScreen();
    
    for (;;) 
    {
        static uint32_t data = 0;
        static uint32_t idx = 0;

        if (hx711GetStatus() == Hx711StatusReady) {
            data = 0;
            if (hx711ReadChannel(Hx711ChannelA128, &data) == Hx711StatusOk) {
                measurments[idx++] = data;
            }
        }
        
        if (idx >= HX711_MEASURMENTS_COUNT) {
            idx = 0;
            if (calibration) {
                qsort(measurments, HX711_MEASURMENTS_COUNT, sizeof(measurments[0]), compare);
                tare = (measurments[4] + measurments[5] + measurments[6]) / 3; // median combined with mean
                printf("Calibration Finished!\n");
                printf("tare == %u\n", tare);
                calibration = false;
            }
            qsort(measurments, HX711_MEASURMENTS_COUNT, sizeof(measurments[0]), compare);
            // for (uint32_t i = 0; i < HX711_MEASURMENTS_COUNT; i++) {
            //     printf("m[%d] == %d ", i, measurments[i]);
            // }
            int32_t mean = (measurments[4] + measurments[5] + measurments[6]) / 3;
            int32_t mass = 0;
            if (tare - mean > 0) {
                mass = (tare - mean) / 196;
            }
            printf("mass == %d gramms\n", mass);
            SSD1306_GotoXY(0, 0);
            SSD1306_Printf(&Font_7x10, "%ld", mass);
            SSD1306_UpdateScreen();
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

    esp_err_t err = i2c_master_init();
    printf("i2c init satus == %d\n", err == ESP_OK ? 1 : 0);

    bool status = SSD1306_Init();
    printf("OLED INIT status == %d\n", status ? 1 : 0);
    SSD1306_UpdateScreen();

    while(1) 
    {
        vTaskDelay(1000);
    }
}
