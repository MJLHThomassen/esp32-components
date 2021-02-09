idf_component_register(
    SRCS 
        "mongoose/mongoose.c"

    INCLUDE_DIRS
        "mongoose"

    REQUIRES
        mbedtls)

target_compile_options(
    ${COMPONENT_LIB}
        PUBLIC
            -Werror=format-truncation=0
            -DMG_ENABLE_MBEDTLS=1
            -DMG_ENABLE_FS=1)
