#ifndef MIC_H
#define MIC_H

#include <stdint.h>
#include <stdbool.h>
#include "hal/gpio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================= CẤU HÌNH PHẦN CỨNG & BUFFER ================= */
#define MIC_SAMPLE_RATE      (16000)      // Tần số lấy mẫu (16kHz)
#define MIC_BUFFER_SIZE      (512)        // Số mẫu PCM thu được mỗi đợt callback

// Cấu hình chân GPIO nối với INMP441 trên ESP32 (Bạn có thể đổi chân tại đây)
#define MIC_I2S_BCK_PIN      (GPIO_NUM_4) // Chân SCK / BCK
#define MIC_I2S_WS_PIN       (GPIO_NUM_5) // Chân WS / LRCK
#define MIC_I2S_DATA_PIN     (GPIO_NUM_18)// Chân SD / DATA

/* ================= TYPEDEFS ================= */
typedef void (*mic_data_callback_t)(const int32_t *p_data, uint16_t length);

/* ================= FUNCTION PROTOTYPES ================= */
/**
 * @brief Khởi tạo I2S driver và tạo Task ngầm đọc dữ liệu Mic.
 * @param cb Hàm callback nhận mảng dữ liệu âm thanh PCM.
 * @return true nếu khởi tạo thành công, false nếu lỗi.
 */
bool mic_init(mic_data_callback_t cb);

/**
 * @brief Hàm handler xử lý dữ liệu. 
 *        (Với ESP32, Task ngầm đã tự xử lý nên hàm này để duy trì tính đóng gói).
 */
void mic_handler(void);

/**
 * @brief Dừng việc thu âm.
 */
void mic_stop(void);

/**
 * @brief Tiếp tục thu âm.
 */
void mic_start(void);

#ifdef __cplusplus
}
#endif

#endif /* MIC_H */