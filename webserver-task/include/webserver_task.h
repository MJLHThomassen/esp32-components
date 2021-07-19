#ifndef WEBSERVER_TASK_H
#define WEBSERVER_TASK_H

#ifdef MONGOOSE7_WEBSERVER_THREAD
#include "mongoose7_webserver_thread.h"
#endif

#define WEBSERVER_TASK_TAG "Webserver"
#define WEBSERVER_TASK_STACK_SIZE_KB 2

void webserver_task_main(void* pvParameters);

#endif // WEBSERVER_TASK_H