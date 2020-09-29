set(srcs    "src/main.cpp"
            "src/sensor_reader.cpp"
            "src/debouncer.cpp"
            "src/config_manager.cpp"
            "src/persistent_store.cpp")

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ""
)
component_compile_options(-std=c++17)