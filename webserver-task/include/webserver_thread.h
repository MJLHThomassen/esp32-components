#ifndef WEBSERVER_THREAD_H
#define WEBSERVER_THREAD_H

#include <stdbool.h>

#define WEBSERVER_THREAD_TAG "Webserver Thread"
#define WEBSERVER_THREAD_STACK_SIZE_KB 8

void start_webserver_thread(void);
void stop_webserver_thread(void);
bool is_webserver_thread_running(void);

#endif // WEBSERVER_THREAD_H