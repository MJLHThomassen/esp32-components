idf_component_register(
    SRCS 
        "mongoose/mongoose.c"

    INCLUDE_DIRS
        "mongoose"

    REQUIRES
        mbedtls
    )

target_compile_options(
    ${COMPONENT_LIB}
        PUBLIC
            -Werror=format-truncation=0
)

target_compile_definitions(
    ${COMPONENT_LIB}
        PUBLIC
            MG_ENABLE_MBEDTLS=1
            MG_ENABLE_FS=1
            MG_ENABLE_DIRECTORY_LISTING=1
)
