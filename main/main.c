#include <stdio.h>
#include "bmi160.h"


int8_t user_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    return 0; 
}

int8_t user_i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
    return 0; 
}


void user_delay_ms(uint32_t ms) {
}

int main(void) {
    // Khai báo đối tượng cảm biến BMI160
    bmi160_dev_t bmi;

    // --- CẤU HÌNH CẢM BIẾN ---
    bmi.dev_addr = 0x68;           // Địa chỉ I2C mặc định của BMI160 (0x68 hoặc 0x69)
    bmi.read     = user_i2c_read;  // Gán hàm đọc phần cứng
    bmi.write    = user_i2c_write; // Gán hàm ghi phần cứng
    bmi.delay    = user_delay_ms;  // Gán hàm delay
    bmi.intf     = BMI160_INTF_UNKNOWN; // Để thư viện tự kiểm tra giao tiếp

    printf("--- BAT DAU KIEM TRA CAM BIEN BMI160 ---\n");

    // 1. Kiểm tra kết nối cảm biến (Tự phát hiện I2C hay SPI)
    bmi160_intf_t intf = bmi160_check_interface(&bmi);
    if (intf == BMI160_INTF_I2C) {
        printf("[OK] Da ket noi voi BMI160 qua chuan I2C!\n");
    } else if (intf == BMI160_INTF_SPI) {
        printf("[OK] Da ket noi voi BMI160 qua chuan SPI!\n");
    } else {
        printf("[ERROR] Khong tim thay cam bien BMI160! Vui long kiem tra lai day noi.\n");
        return -1;
    }

    // 2. Khởi tạo cảm biến (Soft-reset & Bật Accel/Gyro sang Normal Mode)
    if (bmi160_init(&bmi) != 0) {
        printf("[ERROR] Khoi tao BMI160 thoi bat thanh cong!\n");
        return -1;
    }
    printf("[OK] Khoi tao BMI160 thanh cong!\n\n");

    // Khai báo biến chứa dữ liệu thô
    bmi160_raw_data_t accel;
    bmi160_raw_data_t gyro;

    // 3. Vòng lặp đọc dữ liệu liên tục
    while (1) {
        // Đọc đồng thời cả Gia tốc (Accel) và Con quay hồi chuyển (Gyro)
        if (bmi160_read_accel_gyro(&bmi, &accel, &gyro) == 0) {
            
            // --- A. IN DỮ LIỆU THÔ (RAW DATA - 16-bit signed) ---
            printf("[RAW]  ACC [X:%6d Y:%6d Z:%6d] | GYR [X:%6d Y:%6d Z:%6d]\n",
                   accel.x, accel.y, accel.z,
                   gyro.x, gyro.y, gyro.z);

            // --- B. CHUYỂN ĐỔI SANG ĐƠN VỊ THỰC TẾ ---
            // Với dải đo mặc định của BMI160:
            // - Accel (+/- 2g): Độ nhạy là 16384 LSB/g
            // - Gyro (+/- 2000 dps): Độ nhạy là 16.4 LSB/(deg/s)
            
            float acc_x_g = (float)accel.x / 16384.0f;
            float acc_y_g = (float)accel.y / 16384.0f;
            float acc_z_g = (float)accel.z / 16384.0f;

            float gyro_x_dps = (float)gyro.x / 16.4f;
            float gyro_y_dps = (float)gyro.y / 16.4f;
            float gyro_z_dps = (float)gyro.z / 16.4f;

            printf("[PHYS] Gia toc (g):      X: %6.2f | Y: %6.2f | Z: %6.2f\n", acc_x_g, acc_y_g, acc_z_g);
            printf("[PHYS] Vantoc gog (deg/s):X: %6.1f | Y: %6.1f | Z: %6.1f\n", gyro_x_dps, gyro_y_dps, gyro_z_dps);
            printf("--------------------------------------------------------------------------------\n");

        } else {
            printf("[ERROR] Doc du lieu loi!\n");
        }

        // Tạm dừng 500ms trước khi đọc lượt tiếp theo
        bmi.delay(500);
    }

    return 0;
}