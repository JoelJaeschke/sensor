idf_component_register(SRC_DIRS "."
                    PRIV_INCLUDE_DIRS "."
                    PRIV_REQUIRES cmock test_utils)

component_compile_options(-std=c++17)