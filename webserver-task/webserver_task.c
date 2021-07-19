#include <webserver_task.h>

#include <stdbool.h>
#include <string.h>

#include <soc/soc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>

#include <webserver_thread.h>
#include <logger.h>

static SemaphoreHandle_t webserverControlSemaphore = NULL;

static const char* TAG = WEBSERVER_TASK_TAG;

static void wifi_event_sta_disconnected(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    xSemaphoreTake(webserverControlSemaphore, portMAX_DELAY);

    // Stop the webserver thread
    stop_webserver_thread();

    xSemaphoreGive(webserverControlSemaphore);
}

static void ip_event_sta_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    xSemaphoreTake(webserverControlSemaphore, portMAX_DELAY);

    // Start the webserver thread
    start_webserver_thread();

    xSemaphoreGive(webserverControlSemaphore);
}

static void webserver_init()
{
    // Create the semaphore for webserver start/stop control
    webserverControlSemaphore = xSemaphoreCreateMutex();

    // Register events
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_event_sta_disconnected, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_sta_got_ip, NULL);
}

void webserver_task_main(void* pvParameters)
{
    LOG_I(TAG, "Starting task");
    LOG_V(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    webserver_init();

    LOG_D(TAG, "Starting task loop");
    LOG_V(TAG, "Free stack space: %i", uxTaskGetStackHighWaterMark(NULL));
    for (;;)
    {
        vTaskSuspend(NULL);
    }

    vTaskDelete(NULL);
}
