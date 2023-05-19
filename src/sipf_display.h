#ifndef __SIPF_DISPLAY_H__
#define __SIPF_DISPLAY_H__

#include <M5Stack.h>

// 画面設定
void ScreenCOnfig();
// カウンターの画面作成
// 引数：カウンターに表示する値
void IndicateCounterScreen(int counter1, int counter2);

#endif