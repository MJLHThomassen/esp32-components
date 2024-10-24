idf_component_register(
    SRCS 
        "vl53l0x.c"

        "api/core/src/vl53l0x_api_calibration.c"
        "api/core/src/vl53l0x_api_core.c"
        "api/core/src/vl53l0x_api_ranging.c"
        "api/core/src/vl53l0x_api_strings.c"
        "api/core/src/vl53l0x_api.c"

        "api/platform/src/vl53l0x_platform.c"
        "api/platform/src/vl53l0x_esp32_platform.c"

    INCLUDE_DIRS
        "include"
    
    PRIV_INCLUDE_DIRS
        "api/core/include"
        "api/platform/include"
    )
