#ifndef __GPS_H
#define __GPS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <esp_log.h>

// Типы GPS сообщений
enum GPS_TYPE {
  GPS_GSV, // GSV - Детальная информация о спутниках
  GPS_GGA, // GGA - информация о фиксированном решении
  GPS_GSA, // GSA – общая информация о спутниках
  GPS_GLL, // GLL - данные широты и долготы  
  GPS_RMC, // RMC - рекомендованный минимальный набор GPS данных
  GPS_TYPE_UNCNOWN // всё остальное
};

// Радиус ошибки при HDOP=1
#define GPS_HDOP_TO_METERS 7.0

#define PI              3.14159265
#define EARTH_RADIUS_KM 6371.0088
#define RAD_PER_DEG     (PI / 180.0)

// Структура точки GPS, использующаяся в программе
typedef struct {

  // Часть сообщения RMC - координаты, скорость, дата\время
  struct {
    float lat,lon;
    float speed;
    float dir;

    uint16_t hour,minute,second;
    uint16_t day,month,year;
  } rmc;
  
  // Часть сообщения GSA - xDOP, спутники
  struct {
    float hdop, vdop, pdop;
    unsigned int sats[12], satN;
  } gsa;
  
  // Часть сообщения GGA - число спутников и высота
  struct {
    unsigned int sat;
    float height;
  } gga;


  // флаги валидности сообщений
  bool msgs[5];

  // общий флаг валидности
  bool valid;

} gps;

// Структуры сообщений, используемые для разбора
typedef struct {
  uint16_t h,m,s;
  char stat;
  float lat,lon;
  float v;
  float dir;
  uint16_t d,mo,y;

  bool valid;
} gps_rmc;

typedef struct {
  uint8_t des;
  unsigned int sats[12], satN;

  float hdop, pdop, vdop;
} gps_gsa;

typedef struct {
  uint8_t des;
  unsigned int sat;
  float height;
} gps_gga;

typedef struct {
  unsigned int sat;
} gps_gsv;

// Запись символа во внутренний буфер.
// возвращает точку в случае успешного разбора
// и NULL при неуспешном
extern gps* gps_push_sym(char s);

// Обертка для memcpy
extern gps gps_clone(gps* src);

// Расчет расстояния между точками по методу гаверсинусов, в километрах
// http://www.movable-type.co.uk/scripts/latlong.html
// https://github.com/SlashDevin/NeoGPS/blob/master/src/Location.cpp#L80
// https://github.com/SlashDevin/NeoGPS/blob/master/src/Location.h#L55
extern float gps_dist(gps *p1, gps *p2);

#endif
