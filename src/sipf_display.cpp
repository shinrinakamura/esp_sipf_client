#include "sipf_display.h"

// 画面設定
void ScreenCOnfig(){
  M5.Lcd.begin();                   // Initialize M5Stack
  M5.Lcd.init();
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
}

// 画面の作成--------------------------------------------------------
// カウンターの画面作成
void IndicateCounterScreen(int counter1, int counter2){
  M5.Lcd.fillScreen(TFT_BLACK);           // 画面のクリア
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);  
  M5.Lcd.println("communication counter");
  // 文字を聞書いている部分
  M5.Lcd.fillRect(35, 130, 60, 35, TFT_BLUE);      // A
  M5.Lcd.fillRect(130, 130, 60, 35, TFT_BLUE);     // B
  M5.Lcd.fillRect(226, 130, 65, 35, TFT_BLUE);     // C
  // 数字を表示する領域
  M5.Lcd.fillRect(35, 163, 60, 60, TFT_WHITE);      // A
  M5.Lcd.fillRect(130, 163, 60, 60, TFT_WHITE);     // B
  // M5.Lcd.fillRect(226, 163, 65, 60, TFT_WHITE);     // C
  // カウンターに表示する文字
  M5.Lcd.setTextColor(TFT_BLACK, TFT_BLUE); 
  M5.Lcd.drawString("A", 47, 130);
  M5.Lcd.drawString("B", 143, 130);
  M5.Lcd.drawString("clear", 228, 130);
  // カウンターの中の数字
  M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE); 
  M5.Lcd.drawNumber(counter1, 37, 175);
  M5.Lcd.drawNumber(counter2, 133, 175);
}