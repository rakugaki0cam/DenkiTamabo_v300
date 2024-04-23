/*
  servo_common.hpp

  2024.04.23
*/
//tama_pos_tをservo.cppとdisplay.cppの2カ所で使っている。
//servo.hppとdisplay.hpp内でインクルードする
//

#pragma once

//global
typedef enum {
  CENTER1_POS,
  START_POS,
  CENTER2_POS,
  END_POS,
} tama_pos_t;


