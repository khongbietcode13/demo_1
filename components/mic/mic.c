#include "mic.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_log.h"

static const char *TAG = "MIC_INMP441";

#define I2S_PORT_NUM         (I2S_NUM_0)

static mic_data_callback_t g_mic_callback = NULL;
static TaskHandle_t g_mic_task_handle = NULL;
static bool g_is_running = false;

// Buffer tạm để lưu dữ liệu thô đọc từ DMA
static int32_t raw_i2s_buffer[MIC_BUFFER_SIZE];

// Task chạy ngầm trên RTOS để đọc I2S liên tục không gây nghẽn CPU
static void mic_reader_task(void *pvParameters) {
    size_t bytes_read = 0;
    
    while (1) {
        if (g_is_running) {
            // Đọc dữ liệu từ I2S DMA Buffer (Blocking cho đến khi có đủ data)
            esp_err_t result = i2s_read(I2S_PORT_NUM, 
                                       raw_i2s_buffer, 
                                       sizeof(raw_i2s_buffer), 
                                       &bytes_read, 
                                       portMAX_DELAY);

            if (result == ESP_OK && bytes_read > 0) {
                uint16_t samples_read = bytes_read / sizeof(int32_t);
                
                // Gọi callback chuyển dữ liệu về cho app xử lý
                if (g_mic_callback != NULL) {
                    g_mic_callback(raw_i2s_buffer, samples_read);
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

bool mic_init(mic_data_callback_t cb) {
    if (cb == NULL) {
        ESP_LOGE(TAG, "Callback function is NULL!");
        return false;
    }

    g_mic_callback = cb;

    // 1. Cấu hình I2S Driver
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = MIC_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // INMP441 gửi dữ liệu 24-bit trong khung 32-bit
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // Nối L/R xuống GND -> Kênh trái
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = MIC_BUFFER_SIZE,
        .use_apll = false
    };

    // 2. Cấu hình chân GPIO I2S
    i2s_pin_config_t pin_config = {
        .bck_io_num = MIC_I2S_BCK_PIN,
        .ws_io_num = MIC_I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = MIC_I2S_DATA_PIN
    };

    // 3. Install và Cấu hình Driver
    if (i2s_driver_install(I2S_PORT_NUM, &i2s_config, 0, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2S driver");
        return false;
    }

    if (i2s_set_pin(I2S_PORT_NUM, &pin_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set I2S pins");
        return false;
    }

    g_is_running = true;

    // 4. Tạo RTOS Task đọc dữ liệu mic chạy ở Core 1 hoặc 0
    xTaskCreatePinnedToCore(
        mic_reader_task,
        "mic_reader_task",
        4096,
        NULL,
        10, // Độ ưu tiên cao
        &g_mic_task_handle,
        1   // Chạy trên Core 1 của ESP32
    );

    ESP_LOGI(TAG, "Mic INMP441 initialized successfully.");
    return true;
}

void mic_handler(void) {
    // Với ESP32, Task mic_reader_task đã tự động làm toàn bộ công việc đọc dữ liệu background.
    // Hàm này giữ lại để tuân thủ Interface chung của ứng dụng.
}

void mic_start(void) {
    if (!g_is_running) {
        i2s_start(I2S_PORT_NUM);
        g_is_running = true;
    }
}

void mic_stop(void) {
    if (g_is_running) {
        i2s_stop(I2S_PORT_NUM);
        g_is_running = false;
    }
}