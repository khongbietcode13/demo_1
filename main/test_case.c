#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

#include "bmi160.h"

#define I2C_SDA_GPIO       GPIO_NUM_21
#define I2C_SCL_GPIO       GPIO_NUM_22

#define I2C_PORT            I2C_NUM_0
#define I2C_FREQ_HZ         400000

#define BMI160_ADDR_0       0x68
#define BMI160_ADDR_1       0x69

static const char *TAG = "BMI160";

static i2c_master_bus_handle_t i2c_bus;
static i2c_master_dev_handle_t bmi160_i2c_dev;

static int8_t user_i2c_read(
    uint8_t dev_addr,
    uint8_t reg_addr,
    uint8_t *data,
    uint16_t len)
{
    if (data == NULL || len == 0)
        return -1;

    if (dev_addr != BMI160_ADDR_0 && dev_addr != BMI160_ADDR_1)
        return -1;

    uint8_t reg = reg_addr;

    esp_err_t ret = i2c_master_transmit_receive(
        bmi160_i2c_dev,
        &reg,
        1,
        data,
        len,
        1000
    );

    return (ret == ESP_OK) ? 0 : -1;
}

static int8_t user_i2c_write(
    uint8_t dev_addr,
    uint8_t reg_addr,
    const uint8_t *data,
    uint16_t len)
{
    if (data == NULL || len == 0)
        return -1;

    if (dev_addr != BMI160_ADDR_0 && dev_addr != BMI160_ADDR_1)
        return -1;

    uint8_t buffer[256];

    if (len + 1 > sizeof(buffer))
        return -1;

    buffer[0] = reg_addr;

    for (uint16_t i = 0; i < len; i++)
    {
        buffer[i + 1] = data[i];
    }

    esp_err_t ret = i2c_master_transmit(
        bmi160_i2c_dev,
        buffer,
        len + 1,
        1000
    );

    return (ret == ESP_OK) ? 0 : -1;
}

static void user_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static esp_err_t i2c_init(uint8_t bmi_addr)
{
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_io_num = I2C_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus);

    if (ret != ESP_OK)
        return ret;

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = bmi_addr,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    ret = i2c_master_bus_add_device(
        i2c_bus,
        &dev_config,
        &bmi160_i2c_dev
    );

    return ret;
}

void app_main(void)
{
    printf("\n");
    printf("============================\n");
    printf("       BMI160 TEST\n");
    printf("============================\n");

    esp_err_t ret;

    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL_GPIO,
        .sda_io_num = I2C_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ret = i2c_new_master_bus(&bus_config, &i2c_bus);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C init failed: %s", esp_err_to_name(ret));
        return;
    }

    uint8_t bmi_addr = 0;

    ret = i2c_master_probe(i2c_bus, BMI160_ADDR_0, 1000);

    if (ret == ESP_OK)
    {
        bmi_addr = BMI160_ADDR_0;
        ESP_LOGI(TAG, "BMI160 found at 0x68");
    }
    else
    {
        ret = i2c_master_probe(i2c_bus, BMI160_ADDR_1, 1000);

        if (ret == ESP_OK)
        {
            bmi_addr = BMI160_ADDR_1;
            ESP_LOGI(TAG, "BMI160 found at 0x69");
        }
        else
        {
            ESP_LOGE(TAG, "BMI160 not found at 0x68 or 0x69");
            return;
        }
    }

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = bmi_addr,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    ret = i2c_master_bus_add_device(
        i2c_bus,
        &dev_config,
        &bmi160_i2c_dev
    );

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Add BMI160 device failed: %s",
                 esp_err_to_name(ret));
        return;
    }

    bmi160_dev_t bmi;

    bmi.dev_addr = bmi_addr;
    bmi.intf = BMI160_INTF_I2C;
    bmi.read = user_i2c_read;
    bmi.write = user_i2c_write;
    bmi.delay = user_delay_ms;

    ret = bmi160_init(&bmi);

    if (ret != 0)
    {
        ESP_LOGE(TAG, "BMI160 init failed: %d", (int)ret);
        return;
    }

    ESP_LOGI(TAG, "BMI160 initialized successfully");

    while (1)
    {
        bmi160_raw_data_t accel;
        bmi160_raw_data_t gyro;

        ret = bmi160_read_accel_gyro(
            &bmi,
            &accel,
            &gyro
        );

        if (ret == 0)
        {
            printf(
                "ACC: X=%6d  Y=%6d  Z=%6d | "
                "GYRO: X=%6d  Y=%6d  Z=%6d\n",
                accel.x,
                accel.y,
                accel.z,
                gyro.x,
                gyro.y,
                gyro.z
            );
        }
        else
        {
            ESP_LOGE(TAG, "Read BMI160 failed");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}