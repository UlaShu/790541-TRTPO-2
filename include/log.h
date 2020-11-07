#ifndef __LOG_H
#define _LOG_H

// Длина буфера для Live logs
#define LIVE_LOG_BUFFER_LEN 128

// Тэги для логирования
typedef enum {
  TAG_GPS,
}log_tag_num_t;

typedef struct {
	char* tag;
	esp_log_level_t level;
}log_tag_t;

// Имена тегов и уровень логирования
extern char* log_name(log_tag_num_t tag);

#endif
