#ifndef BMI160_H
#define BMI160_H

#include <stdint.h>
#include <stdbool.h>

// Thông tin định danh cảm biến
#define BMI160_CHIP_ID          0xD1

// Địa chỉ các thanh ghi chính (Register Map)
#define BMI160_REG_CHIP_ID      0x00
#define BMI160_REG_ERR_REG      0x02
#define BMI160_REG_PMU_STATUS   0x03
#define BMI160_REG_GYRO_DATA    0x0C
#define BMI160_REG_ACCEL_DATA   0x12
#define BMI160_REG_ACC_CONF     0x40
#define BMI160_REG_ACC_RANGE    0x41
#define BMI160_REG_GYR_CONF     0x42
#define BMI160_REG_GYR_RANGE    0x43
#define BMI160_REG_IF_CONF      0x6B
#define BMI160_REG_CMD          0x7E

// Các lệnh điều khiển (PMU Commands)
#define BMI160_CMD_ACCEL_NORMAL 0x11
#define BMI160_CMD_GYRO_NORMAL  0x15
#define BMI160_CMD_SOFT_RESET   0xB6

// Loại chuẩn giao tiếp
typedef enum {
    BMI160_INTF_UNKNOWN = 0,
    BMI160_INTF_I2C,
    BMI160_INTF_SPI
} bmi160_intf_t;

// Cấu trúc dữ liệu 3 trục thô (Raw Data)
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} bmi160_raw_data_t;

// Định nghĩa con trỏ hàm cho phần cứng (Hardware Abstraction Layer)
typedef int8_t (*bmi160_read_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef int8_t (*bmi160_write_fptr_t)(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, uint16_t len);
typedef void (*bmi160_delay_fptr_t)(uint32_t ms);

// Cấu trúc quản lý thiết bị BMI160
typedef struct {
    uint8_t dev_addr;           // Địa chỉ I2C hoặc chân CS của SPI
    bmi160_intf_t intf;         // Chuẩn giao tiếp hiện tại (I2C hoặc SPI)
    bmi160_read_fptr_t read;    // Hàm đọc phần cứng
    bmi160_write_fptr_t write;  // Hàm ghi phần cứng
    bmi160_delay_fptr_t delay;  // Hàm delay ms
} bmi160_dev_t;

/* --- KHAI BÁO CÁC HÀM NGUYÊN MẪU --- */

// Kiểm tra giao tiếp đang dùng là I2C hay SPI
bmi160_intf_t bmi160_check_interface(bmi160_dev_t *dev);

// Khởi tạo cảm biến
int8_t bmi160_init(bmi160_dev_t *dev);

// Hàm đọc dữ liệu trực tiếp qua I2C hoặc SPI
int8_t bmi160_read_i2c(bmi160_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len);
int8_t bmi160_read_spi(bmi160_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len);

// Đọc dữ liệu Gia tốc (Accelerometer)
int8_t bmi160_read_accel(bmi160_dev_t *dev, bmi160_raw_data_t *accel);

// Đọc dữ liệu Con quay hồi chuyển (Gyroscope)
int8_t bmi160_read_gyro(bmi160_dev_t *dev, bmi160_raw_data_t *gyro);

// Đọc đồng thời cả Accel và Gyro
int8_t bmi160_read_accel_gyro(bmi160_dev_t *dev, bmi160_raw_data_t *accel, bmi160_raw_data_t *gyro);

#endif // BMI160_H