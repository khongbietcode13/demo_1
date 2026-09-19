#include "bmi160.h"

/**
 * @brief Đọc dữ liệu qua chuẩn I2C
 */
int8_t bmi160_read_i2c(bmi160_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    if (!dev || !dev->read) return -1;
    return dev->read(dev->dev_addr, reg_addr, data, len);
}

/**
 * @brief Đọc dữ liệu qua chuẩn SPI (Tự động set bit MSB lên 1 theo chuẩn BMI160 SPI Read)
 */
int8_t bmi160_read_spi(bmi160_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    if (!dev || !dev->read) return -1;
    uint8_t spi_reg = reg_addr | 0x80; // MSB = 1 khi đọc SPI
    return dev->read(dev->dev_addr, spi_reg, data, len);
}

/**
 * @brief Kiểm tra xem giao tiếp hiện tại kết nối thành công qua I2C hay SPI
 */
bmi160_intf_t bmi160_check_interface(bmi160_dev_t *dev) {
    if (!dev || !dev->read) return BMI160_INTF_UNKNOWN;

    uint8_t chip_id = 0;

    // Thử đọc qua I2C trước
    if (bmi160_read_i2c(dev, BMI160_REG_CHIP_ID, &chip_id, 1) == 0) {
        if (chip_id == BMI160_CHIP_ID) {
            dev->intf = BMI160_INTF_I2C;
            return BMI160_INTF_I2C;
        }
    }

    // Nếu không phản hồi I2C, thử đọc qua SPI
    if (bmi160_read_spi(dev, BMI160_REG_CHIP_ID, &chip_id, 1) == 0) {
        if (chip_id == BMI160_CHIP_ID) {
            dev->intf = BMI160_INTF_SPI;
            return BMI160_INTF_SPI;
        }
    }

    dev->intf = BMI160_INTF_UNKNOWN;
    return BMI160_INTF_UNKNOWN;
}

/**
 * @brief Hàm đọc thanh ghi nội bộ phụ thuộc vào giao tiếp active
 */
static int8_t bmi160_read_reg(bmi160_dev_t *dev, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    if (dev->intf == BMI160_INTF_I2C) {
        return bmi160_read_i2c(dev, reg_addr, data, len);
    } else if (dev->intf == BMI160_INTF_SPI) {
        return bmi160_read_spi(dev, reg_addr, data, len);
    }
    return -1;
}

/**
 * @brief Hàm ghi thanh ghi nội bộ phụ thuộc vào giao tiếp active
 */
static int8_t bmi160_write_reg(bmi160_dev_t *dev, uint8_t reg_addr, const uint8_t *data, uint16_t len) {
    if (!dev || !dev->write) return -1;
    uint8_t reg = reg_addr;
    if (dev->intf == BMI160_INTF_SPI) {
        reg = reg_addr & 0x7F; // MSB = 0 khi ghi qua SPI
    }
    return dev->write(dev->dev_addr, reg, data, len);
}

/**
 * @brief Khởi tạo BMI160 (Xác định giao tiếp & Bật nguồn cảm biến)
 */
int8_t bmi160_init(bmi160_dev_t *dev) {
    if (!dev) return -1;

    // Tự động kiểm tra giao tiếp nếu chưa cấu hình
    if (dev->intf == BMI160_INTF_UNKNOWN) {
        if (bmi160_check_interface(dev) == BMI160_INTF_UNKNOWN) {
            return -2; // Không phản hồi từ chip
        }
    }

    // Lệnh Soft Reset cảm biến
    uint8_t cmd = BMI160_CMD_SOFT_RESET;
    bmi160_write_reg(dev, BMI160_REG_CMD, &cmd, 1);
    if (dev->delay) dev->delay(15);

    // Chuyển Accelerometer sang Normal Mode
    cmd = BMI160_CMD_ACCEL_NORMAL;
    bmi160_write_reg(dev, BMI160_REG_CMD, &cmd, 1);
    if (dev->delay) dev->delay(10);

    // Chuyển Gyroscope sang Normal Mode
    cmd = BMI160_CMD_GYRO_NORMAL;
    bmi160_write_reg(dev, BMI160_REG_CMD, &cmd, 1);
    if (dev->delay) dev->delay(80);

    return 0;
}

/**
 * @brief Đọc giá trị Gia tốc (Accelerometer - X, Y, Z)
 */
int8_t bmi160_read_accel(bmi160_dev_t *dev, bmi160_raw_data_t *accel) {
    if (!accel) return -1;
    uint8_t buf[6];

    if (bmi160_read_reg(dev, BMI160_REG_ACCEL_DATA, buf, 6) != 0) {
        return -1;
    }

    accel->x = (int16_t)((buf[1] << 8) | buf[0]);
    accel->y = (int16_t)((buf[3] << 8) | buf[2]);
    accel->z = (int16_t)((buf[5] << 8) | buf[4]);

    return 0;
}

/**
 * @brief Đọc giá trị Con quay hồi chuyển (Gyroscope - X, Y, Z)
 */
int8_t bmi160_read_gyro(bmi160_dev_t *dev, bmi160_raw_data_t *gyro) {
    if (!gyro) return -1;
    uint8_t buf[6];

    if (bmi160_read_reg(dev, BMI160_REG_GYRO_DATA, buf, 6) != 0) {
        return -1;
    }

    gyro->x = (int16_t)((buf[1] << 8) | buf[0]);
    gyro->y = (int16_t)((buf[3] << 8) | buf[2]);
    gyro->z = (int16_t)((buf[5] << 8) | buf[4]);

    return 0;
}

/**
 * @brief Đọc đồng thời dữ liệu Accel và Gyro trong 1 lần đọc liên tiếp (12 bytes)
 */
int8_t bmi160_read_accel_gyro(bmi160_dev_t *dev, bmi160_raw_data_t *accel, bmi160_raw_data_t *gyro) {
    if (!accel || !gyro) return -1;
    uint8_t buf[12]; // Gyro (0x0C -> 0x11) + Accel (0x12 -> 0x17)

    if (bmi160_read_reg(dev, BMI160_REG_GYRO_DATA, buf, 12) != 0) {
        return -1;
    }

    gyro->x = (int16_t)((buf[1] << 8) | buf[0]);
    gyro->y = (int16_t)((buf[3] << 8) | buf[2]);
    gyro->z = (int16_t)((buf[5] << 8) | buf[4]);

    accel->x = (int16_t)((buf[7] << 8) | buf[6]);
    accel->y = (int16_t)((buf[9] << 8) | buf[8]);
    accel->z = (int16_t)((buf[11] << 8) | buf[10]);

    return 0;
}