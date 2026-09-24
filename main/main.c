#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "dsps_fft2r.h"   // Xử lý FFT
#include "dsps_dotprod.h" // Tích vô hướng
#include "dsps_math.h"    // Các hàm toán học tối ưu
#include "esp_dsp.h"      // Các hàm DSP tối ưu

#include "bmi160.h"
#include "mic.h"
 int app_main(void)
{

}