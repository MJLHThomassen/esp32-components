#include "logger_service.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sdkconfig.h>
#include <slist.h>

char logger_service_timestamp[20] = "";

struct sink_s
{
    logger_sink_t callback;
    void* user_data;
};

static slist sinks;

void logger_service_init(void)
{
    // ESP_LOGI("logger", "Initializing Logger");
    slist_new(&sinks);
}

int logger_service_log(logger_service_loglevel_t level, const char* format, ...)
{
    va_list list;
    va_start(list, format);
    int len = logger_service_vlog(level, format, list);
    va_end(list);

    return len;
}

int logger_service_vlog(logger_service_loglevel_t level, const char* format, va_list vlist)
{
    if(level > CONFIG_LOG_DEFAULT_LEVEL)
    {
        // Don't log current level
        return 0;
    }

    char* str = (char*) calloc(512, sizeof(char));

    if(str == NULL)
    {
        // ESP_LOGW("Logger", "Could not allocate memory for writing to logger service sinks");
        return -1;
    }

    // Update timestamp so it has the correct value if it is present in vlist
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    if(now != (time_t)(-1) && timeinfo.tm_year >= (2020 - 1900))
    {
        // Time is properly set, print it
        strftime(logger_service_timestamp, 20, "%F %T", gmtime(&now));
    }
    else
    {
        // Time is not properly set, print the clock
        snprintf(logger_service_timestamp, 20, "%li", clock());
    }

    // Format string
    int len = vsnprintf(str, 512, format, vlist);

    // Print to sinks 
    sink_handle_t sink;
    slist_iter sinksIter;
    if(slist_iter_new(sinks, &sinksIter) != UTIL_ERR_ALLOC)
    {
        while(slist_iter_next(sinksIter, (void**)(&sink)) != UTIL_ITER_END)
        {
            // Call the sink
            if(sink != NULL)
            {
                sink->callback(str, len, sink->user_data);
            }
        }
        
        slist_iter_delete(sinksIter);
    }
    else
    {
        // ESP_LOGW("Logger", "Could not allocate memory for itterating logger service sinks");
    }
    
    free(str);

    return len;
}

sink_handle_t logger_service_register_sink(logger_sink_t callback, void* user_data)
{
    sink_handle_t newSink = (sink_handle_t)malloc(sizeof(*newSink));

    if(newSink == NULL)
    {
        // ESP_LOGW("Logger", "Could not allocate memory for adding sink");
        return NULL;
    }

    newSink->callback = callback;
    newSink->user_data = user_data;

    slist_add(sinks, (void*)newSink);

    return newSink;
}

void logger_service_unregister_sink(sink_handle_t handle)
{
    slist_remove(sinks, (void*)handle);
}
