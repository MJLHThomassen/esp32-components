idf_component_register(
	SRCS 
		"esp_crypto.c"
		"mongoose/mongoose.c"

	INCLUDE_DIRS
		"${COMPONENT_DIR}"
		"mongoose"
		
	REQUIRES
		"mbedtls")

target_compile_options(${COMPONENT_LIB} PUBLIC -Werror=format-truncation=0 -DMG_LOCALS=1)
