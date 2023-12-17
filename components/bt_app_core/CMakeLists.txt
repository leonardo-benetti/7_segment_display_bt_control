idf_component_register(SRCS "bt_app_core.c"
                    INCLUDE_DIRS "include")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
