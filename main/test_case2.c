#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mic.h"

static const char *TAG = "MAIN_APP";

// Hàm Callback nhận dữ liệu PCM từ mic
void on_audio_data_received(const int32_t *p_data, uint16_t length) {
    int32_t max_val = 0;

    // INMP441 trả về dữ liệu 24-bit căn trái trong khung 32-bit (dịch phải 8 bit để lấy giá trị thực)
    for (uint16_t i = 0; i < length; i++) {
        int32_t val = p_data[i] >> 8;
        if (val < 0) val = -val;
        if (val > max_val) max_val = val;
    }

    // In biên độ cực đại âm thanh thu được ra Console/Serial Plotter
    ESP_LOGI(TAG, "Audio Chunk Received: %d samples | Peak Amplitude: %ld", length, max_val);
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting Micro INMP441 Demo App...");

    // 1. Khởi tạo mic và truyền callback
    if (!mic_init(on_audio_data_received)) {
        ESP_LOGE(TAG, "Failed to initialize microphone!");
        return;
    }

    // 2. Vòng lặp chính của ứng dụng
    while (1) {
        // Tự động kiểm tra / thực thi các tác vụ background nếu có
        mic_handler();

        // Delay 10ms để không bị báo lỗi ngắt Watchdog Timer (WDT) của ESP32
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}