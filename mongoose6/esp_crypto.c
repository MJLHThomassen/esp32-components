#include "mongoose/mongoose.h"

#if MG_ENABLE_SSL

#include "esp_system.h"

int mg_ssl_if_mbed_random(void *ctx, unsigned char *buf, size_t len) 
{  
  (void) ctx;
  uint32_t rnd;
  
  for(int i = 0; i < len; i++)
  {
    if(i % 4 == 0)
    {
      rnd = esp_random();
    }

    buf[i] = ((unsigned char*)(&rnd))[i % 4];
  }

  return 0;
}

#endif /* MG_ENABLE_SSL */
