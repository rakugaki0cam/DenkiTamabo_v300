/*
  sd_common.hpp

  2024.04.26
*/
//wifi_stat_tをsd.cppとdisplay.cppの2カ所で使っている。
//sd.hppとdisplay.hpp内でインクルードする
//

#pragma once

//global
typedef enum {
  TEXT_WIFI,
  TEXT_DOT,
  TEXT_DOT_NONE,
  TEXT_WIFI_CONECT,
  TEXT_WIFI_TIMEOUT,
  TEXT_IP,
  TEXT_NTP,
  TEXT_RTC,
  TEXT_DATE_TIME,
  TEXT_NTP_TIMEOUT,
  TEXT_WIFI_OK,
} wifi_stat_t;

typedef enum {
  BTS_INIT,
  BTS_OK,
  BTS_TIMEOUT,
  BTS_PORT1,
  BTS_PORT2,
} bts_stat_t;