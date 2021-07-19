#include <mongoose7_webserver_thread.h>

#include <stdbool.h>
#include <string.h>

#include <soc/soc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_event.h>

#include <mongoose.h>
#include <logger.h>

#ifndef WEBSERVER_ROOT
#define WEBSERVER_ROOT NULL
#endif

#define STACK_KB 1024 / sizeof(portSTACK_TYPE) // The size of a Kilobyte of stack memory

static TaskHandle_t webserverThreadHandle = NULL;
static TaskHandle_t stoppingTaskHandle = NULL;

static struct mg_mgr gManager;
static mongoose7_webserver_thread_event_handler_t gHandler = NULL;

static const char* TAG = WEBSERVER_THREAD_TAG;

#ifdef WEBSERVER_ROOT_ON_SPIFFS
// SPIFFS is flat, so tell Mongoose that the FS root is a directory
// This cludge is not required for filesystems with directory support
bool mg_is_dir(const char *path)
{
    return strcmp(path, WEBSERVER_ROOT) == 0;
}
#endif

static void mongoose7_event_handler(struct mg_connection *c, int ev, void *evData, void *fnData)
{
    bool handled = false;
    if(gHandler)
    {
        handled = gHandler(c, ev, evData, fnData);
    }

    if(!handled && WEBSERVER_ROOT && ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message* hm = (struct mg_http_message *) evData;
        struct mg_http_serve_opts httpServeOpts = {
            .root_dir = WEBSERVER_ROOT
        };

        LOG_D(TAG, "Serving http");
        mg_http_serve_dir(c, hm, &httpServeOpts);
    }

    // switch(ev)
    // {
    //     case MG_EV_HTTP_MSG:
    //     {
    //         struct mg_http_message* hm = (struct mg_http_message *) evData;

    //         if (mg_http_match_uri(hm, "/websocket")) 
    //         {
    //             // Upgrade to websocket. From now on, a connection is a full-duplex
    //             // Websocket connection, which will receive MG_EV_WS_MSG events.
    //             mg_ws_upgrade(c, hm, NULL);
    //         } 
    //         else if(mg_http_match_uri(hm, "/api/f1"))
    //         {
    //             mg_http_reply(c, 200, "", "{\"result\": %d}\n", 123);
    //         }
    //         else if (mg_http_match_uri(hm, "/api/f2/*")) 
    //         {
    //             mg_http_reply(c, 200, "", "{\"result\": \"%.*s\"}\n", (int) hm->uri.len, hm->uri.ptr);
    //         }
    //         else
    //         {
    //             LOG_I(TAG, "Serving http");
    //             mg_http_serve_dir(c, hm, &httpServeOpts);
    //         }
    //         break;
    //     }
        
    //     case MG_EV_WS_MSG:
    //     {
    //         struct mg_ws_message* wm = (struct mg_ws_message *) evData;
    //         mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
    //         mg_iobuf_delete(&c->recv, c->recv.len);
    //     }
    // }
}

static void mongoose7_webserver_thread(void* pvParameters)
{
    LOG_I(TAG, "Webserver polling loop started");

    for (;;)
    {
        // Webserver event loop
        mg_mgr_poll(&gManager, 100);

        // Check to see if we need to stop
        uint32_t stop = ulTaskNotifyTake(pdTRUE, 0);
        if( stop == 1 )
        {
            break;
        }
    }
    LOG_I(TAG, "Webserver polling loop stopped");

    // Notify the stopping task we have stopped
    xTaskNotifyGive(stoppingTaskHandle);

    // Delete the task before returning
    vTaskDelete(NULL);
}

void start_webserver_thread(void)
{
    if(!is_webserver_thread_running())
    {
        LOG_I(TAG, "Starting webserver thread");
        
        // Initialise the webserver manager
        mg_mgr_init(&gManager);

        // Set up webserver connection
        LOG_I(TAG, "Starting http server on: '%s'", WEBSERVER_HOST);
        mg_http_listen(&gManager, WEBSERVER_HOST, mongoose7_event_handler, &gManager);

        // Start the webserver thread
        BaseType_t taskCreateResult = xTaskCreatePinnedToCore(
            mongoose7_webserver_thread,
            WEBSERVER_THREAD_TAG,
            WEBSERVER_THREAD_STACK_SIZE_KB * STACK_KB,
            NULL,
            tskIDLE_PRIORITY+12,
            &webserverThreadHandle,
            APP_CPU_NUM);

        if(taskCreateResult == pdPASS)
        {
            LOG_I(TAG, "Webserver thread started");
        }
        else
        {
            LOG_E(TAG, "Webserver thread could not be started");
            
            // Free webserver resources
            mg_mgr_free(&gManager);

            webserverThreadHandle = NULL;
        }
    }
    else
    {
        LOG_I(TAG, "Webserver thread already running");
    }
}

void stop_webserver_thread(void)
{
    if(is_webserver_thread_running())
    {
        LOG_I(TAG, "Stopping webserver thread");

        // Get calling task handle
        stoppingTaskHandle = xTaskGetCurrentTaskHandle();

        // Notify webserver thread to stop
        xTaskNotifyGive(webserverThreadHandle);

        // Wait for websever thread to be stopped
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Free webserver resources
        mg_mgr_free(&gManager);

        webserverThreadHandle = NULL;
        stoppingTaskHandle = NULL;
        
        LOG_I(TAG, "Webserver thread stopped");
    }
    else
    {
        LOG_I(TAG, "Webserver thread not running");
    }
}

bool is_webserver_thread_running(void)
{
    return webserverThreadHandle != NULL;
}

void mongoose7_webserver_thread_set_event_handler(mongoose7_webserver_thread_event_handler_t handler)
{
    gHandler = handler;
}