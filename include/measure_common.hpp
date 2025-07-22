/*
  measure_common.hpp

  2024.04.27
*/
//noz_stat_tをmeasure.cppとdisplay.cppの2カ所で使っている。
//measure.hppとdisplay.hpp内でインクルードする

#pragma once

//global
typedef enum {
  NOZ_TITLE,
  NOZ_EXP1,
  NOZ_ABORT,
  NOZ_MEAS,
  NOZ_EXP3,
  NOZ_NOZZLE_OK,
  NOZ_PACKING,
  NOZ_PACKING_OK,
  NOZ_NOZZLE,
  NOZ_DETECT,
  NOZ_TUMADUKI,
  NOZ_ERR,
  NOZ_OK,
  NOZ_RESET,
  NOZ_MARK,
  NOZ_EN,
  NOZ_DIS,
} noz_stat_t;

