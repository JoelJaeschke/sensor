idf_component_register(
    SRCS "main.cpp" "sensor_reader.cpp" "debouncer.cpp" "pwm_driver.cpp" "config_manager.cpp" "persistent_store.cpp"
    INCLUDE_DIRS ""
)
component_compile_options(-std=c++17)