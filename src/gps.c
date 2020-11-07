#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "gps.h"

#define TAG_GPS "GPS"

uint8_t gps_get_type(char *buf) {
  if (strncmp(&buf[3], "GGA",3) == 0) {
    return GPS_GGA;
  } else if (strncmp(&buf[3], "GSA",3) == 0) {
    return GPS_GSA;
  } else if (strncmp(&buf[3], "RMC",3) == 0) {
    return GPS_RMC;
  } else if (strncmp(&buf[3], "GLL",3) == 0) {
    return GPS_GLL;
  } else if (strncmp(&buf[3], "GSV",3) == 0) {
    return GPS_GSV;
  }

  return GPS_TYPE_UNCNOWN;
}

char* gps_shift(char *src) {
  char *buf=src;
  while( *buf != 0 && *buf != ',')buf++;
  if(*buf == 0) return NULL;
  return ++buf;
}

float lat_lon(float raw) {
  float deg = floor( raw/100.0 );
  float min = raw-(deg*100.0);
  return deg + ( raw > 0 ? (min/60.0) : (-min/60.0) );
}

gps_gsv *gps_parse_gsv(char *src) {
  static gps_gsv gsv;

  uint8_t i=0;
  char *buf = src;

  while (buf != NULL) {
    switch (i){
      case 3:
        sscanf(buf, "%u", &gsv.sat);
        break;
    }
    buf = gps_shift(buf);
    i++;
  }
  return &gsv;
}

gps_gga *gps_parse_gga(char *src) {
  static gps_gga gga;
  uint8_t i=0;
  char *buf = src;

  while (buf != NULL) {
    switch (i){
      case 6: // StandAlone решение
        sscanf(buf, "%c", &gga.des);
        gga.des -= '0';
      break;
      case 7: 
        sscanf(buf, "%u", &gga.sat);
        break;
      case 9:
        sscanf(buf, "%f", &gga.height);
        break;
    }
    buf = gps_shift(buf);
    i++;
  }

  return &gga;
}

gps_gsa *gps_parse_gsa(char *src) {
  static gps_gsa gsa;
  uint8_t i=0;
  char *buf = src;

  while (buf != NULL) {
    if(i>=3 && i<15) { // satellites
      sscanf(buf, "%u", &gsa.sats[i-3]);
      gsa.satN ++;
    }
    switch (i){
    case 2: // тип решения, 3D решение (1 – нет решения, 2 – 2D решение, 3 – 3D решение)
    sscanf(buf, "%c", &gsa.des);
    gsa.des -= '0';
    break;
    case 15: // PDOP
    sscanf(buf, "%f", &gsa.pdop);
    break;
    case 16: // HDOP
    sscanf(buf, "%f", &gsa.hdop);
    break;
    case 17: // VDOP
    sscanf(buf, "%f", &gsa.vdop);
    break;
    }
    buf = gps_shift(buf);
    i++;
  }

  return &gsa;
}

gps_rmc *gps_parse_rmc(char *src) {
  static gps_rmc rmc;
  uint8_t i=0;
  unsigned int time;
  char *buf = src;
  float raw;

  while (buf != NULL) {
    switch (i){
    case 0:break; //type
    case 1: // time
      sscanf(buf, "%u", &time);
      rmc.s = time%100; time = time/100;
      rmc.m = time%100; time = time/100;
      rmc.h = time;
      break;
    case 2: // flag
      sscanf(buf, "%c", &rmc.stat);
      rmc.valid = (rmc.stat == 'A' ? true : false);
      break;
    case 3: // lat
      sscanf(buf, "%f", &raw);
      rmc.lat = lat_lon(raw);
    break;
    case 4: // N \ S
      if (*buf == 'S' || *buf == 's') rmc.lat *=-1;
    break;
    case 5: // lon
      sscanf(buf, "%f", &raw);
      rmc.lon = lat_lon(raw);
    break;
    case 6: // E \ W
      if (*buf == 'W' || *buf == 'w') rmc.lon *=-1;
    break;
    case 7: // speed, knots converts to m/s
      sscanf(buf, "%f", &rmc.v);
      rmc.v *= 0.514444;
      rmc.v = (rmc.v < 0.5 ? 0 : rmc.v);
    break;
    case 8: // direction
      if (rmc.v > 0) sscanf(buf, "%f", &rmc.dir);
    break;
    case 9: // date
      sscanf(buf, "%u", &time);
      rmc.y = time%100; time = time/100;
      rmc.mo = time%100; time = time/100;
      rmc.d = time;
    break;
    }
    
    buf = gps_shift(buf);
    i++;
  } 

  return &rmc;   
}

gps *gps_parse_message(char *gps_buf) {
    static gps point;

    ESP_LOGV(TAG_GPS, "%s", gps_buf);
    
    uint8_t type = gps_get_type(gps_buf);
    gps_rmc *rmc;
    gps_gga *gga;
    gps_gsa *gsa;

    if(point.valid && type == GPS_TYPE_UNCNOWN) // if point valid & got new message 
      for(uint8_t i=0;i<sizeof(point.msgs); i ++) 
        point.msgs[i] = false;

    switch (type) {
    case GPS_GLL:
    case GPS_GSV:
      point.msgs[type] = true;
    break;
    case GPS_RMC:
      ESP_LOGD(TAG_GPS, "%s", gps_buf);

      rmc = gps_parse_rmc(gps_buf);
      point.msgs[GPS_RMC] = false;
      if((*rmc).valid ) {
        point.rmc.lat = (*rmc).lat;
        point.rmc.lon = (*rmc).lon;
        point.rmc.speed = (*rmc).v;
        point.rmc.dir = (*rmc).dir;

        point.rmc.hour = (*rmc).h; point.rmc.minute = (*rmc).m;  point.rmc.second = (*rmc).s;
        point.rmc.day  = (*rmc).d; point.rmc.month  = (*rmc).mo; point.rmc.year = (*rmc).y;

        point.msgs[GPS_RMC] = true;
      }
    break;
    case GPS_GSA:
      ESP_LOGD(TAG_GPS, "%s", gps_buf);

      gsa = gps_parse_gsa(gps_buf);
      point.gsa.hdop = (*gsa).hdop;
      point.gsa.pdop = (*gsa).pdop;
      point.gsa.vdop = (*gsa).vdop;

      point.msgs[GPS_GSA] = ((*gsa).des == 1 ? false : true);

      point.gsa.satN = (*gsa).satN;
      memcpy(point.gsa.sats, &((*gsa).sats), sizeof(point.gsa.sats));
    break;
    case GPS_GGA:
      ESP_LOGD(TAG_GPS, "%s", gps_buf);
      
      gga = gps_parse_gga(gps_buf);
      point.msgs[GPS_GGA] = ((*gga).des == 0 ? false : true);

      point.gga.sat = (*gga).sat;
      point.gga.height = (*gga).height;
    break;
    }

    point.valid = true;
    for(uint8_t i=0;i<sizeof(point.msgs); i ++) 
      point.valid = (point.msgs[i] && point.valid);

    return &point;
}

gps* gps_push_sym(char s) {
    static char gps_buf[100];
    static uint8_t gps_index=0;

    if (s == 13) {
      gps_buf[gps_index] = 0;
      gps_index=0;

      return gps_parse_message(gps_buf);

    } else if (s == '$' || gps_index > 0) {
      if(s == '$')gps_index=0;

      gps_buf[gps_index] = s;
      gps_index++;
    }

    return NULL;
}

// ToDo: implement
gps *gps_push_string(char *buf, size_t len) {
  return NULL;
}

gps gps_clone(gps* src) {
    static gps point;
    if (src != NULL)memcpy((void*)&point, (void*)src, sizeof(point));

    return point;
}

float gps_dist(gps *p1, gps *p2) {
  float dLat = (p2->rmc.lat - p1->rmc.lat) * RAD_PER_DEG;
  float dLon = (p2->rmc.lon - p1->rmc.lon) * RAD_PER_DEG;
  float x    = dLon * cos( p1->rmc.lat  * RAD_PER_DEG + dLat/2 );
  return sqrt( x*x + dLat*dLat ) * EARTH_RADIUS_KM;
}
