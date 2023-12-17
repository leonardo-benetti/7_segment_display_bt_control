idf_component_register(SRCS "bt_app_av.c"
                    REQUIRES bt
                    INCLUDE_DIRS "include" "../bt_app_core/include")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
