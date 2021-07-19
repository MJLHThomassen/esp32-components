#ifndef MONGOOSE7_WEBSERVER_THREAD_H
#define MONGOOSE7_WEBSERVER_THREAD_H

#include <webserver_thread.h>
#include <mongoose.h>

#include <stdbool.h>

typedef bool (*mongoose7_webserver_thread_event_handler_t)(
    struct mg_connection* c,
    int ev,
    void *evData,
    void *fnData);

void mongoose7_webserver_thread_set_event_handler(mongoose7_webserver_thread_event_handler_t handler);

#endif // MONGOOSE7_WEBSERVER_THREAD_H