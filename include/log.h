#ifndef __LOG_H
#define _LOG_H

// Длина буфера для Live logs
#define LIVE_LOG_BUFFER_LEN 128

// Тэги для логирования
typedef enum {
  TAG_GPS,
}log_tag_num_t;

extern char* log_name(log_tag_num_t tag);

#endif
