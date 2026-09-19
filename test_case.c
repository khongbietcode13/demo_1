#include "bmi160.h"

// Tạo instance thiết bị
bmi160_dev_t bmi;

void main(void) {
    // 1. Gán con trỏ hàm I2C/SPI phần cứng của Vi điều khiển (Ví dụ STM32/ESP32)
    bmi.dev_addr = 0x68; // Địa chỉ I2C (hoặc chân CS nếu dùng SPI)
    bmi.read     = user_i2c_read;
    bmi.write    = user_i2c_write;
    bmi.delay    = user_delay_ms;
    bmi.intf     = BMI160_INTF_UNKNOWN; // Để thư viện tự check

    // 2. Kiểm tra chuẩn giao tiếp đang dùng
    bmi160_intf_t mode = bmi160_check_interface(&bmi);
    if (mode == BMI160_INTF_I2C) {
        // Đã xác nhận là I2C
    } else if (mode == BMI160_INTF_SPI) {
        // Đã xác nhận là SPI
    }

    // 3. Khởi tạo cảm biến (Gửi các lệnh Power mode)
    bmi160_init(&bmi);

    // 4. Đọc dữ liệu
    bmi160_raw_data_t accel, gyro;
    
    // Đọc riêng Accel
    bmi160_read_accel(&bmi, &accel);
    
    // Đọc đồng thời cả Accel và Gyro
    bmi160_read_accel_gyro(&bmi, &accel, &gyro);
}