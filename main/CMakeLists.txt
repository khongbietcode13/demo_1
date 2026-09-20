idf_component_register(
    SRCS "main.c" "test_case.c" "test_case2.c"
    INCLUDE_DIRS "."
    REQUIRES bmi160 esp_driver_i2c
)