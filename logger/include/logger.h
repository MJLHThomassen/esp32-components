#ifndef LOG_H
#define LOG_H

#ifdef USE_LOGGER_SERVICE

    // Use the logger service for advanced logging scenarios
    #include "logger_service.h"

    #define LOG_E(tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_ERROR,   LOGGER_SERVICE_FORMAT(E, format), logger_service_timestamp, tag, ##__VA_ARGS__)
    #define LOG_W(tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_WARN,    LOGGER_SERVICE_FORMAT(W, format), logger_service_timestamp, tag, ##__VA_ARGS__)
    #define LOG_I(tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_INFO,    LOGGER_SERVICE_FORMAT(I, format), logger_service_timestamp, tag, ##__VA_ARGS__)
    #define LOG_D(tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_DEBUG,   LOGGER_SERVICE_FORMAT(D, format), logger_service_timestamp, tag, ##__VA_ARGS__)
    #define LOG_V(tag, format, ... ) logger_service_log(LOGGER_SERVICE_LOGLEVEL_VERBOSE, LOGGER_SERVICE_FORMAT(V, format), logger_service_timestamp, tag, ##__VA_ARGS__)

#else

    // Forward logging to the standard ESP log utilities
    #include <esp_log.h>

    #define LOG_E(tag, format, ... ) ESP_LOGE(tag, format, ##__VA_ARGS__)
    #define LOG_W(tag, format, ... ) ESP_LOGW(tag, format, ##__VA_ARGS__)
    #define LOG_I(tag, format, ... ) ESP_LOGI(tag, format, ##__VA_ARGS__)
    #define LOG_D(tag, format, ... ) ESP_LOGD(tag, format, ##__VA_ARGS__)
    #define LOG_V(tag, format, ... ) ESP_LOGV(tag, format, ##__VA_ARGS__)

#endif // USE_LOGGER_SERVICE

#endif // LOG_H