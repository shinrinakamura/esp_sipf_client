/*
環境センサーを使用したサンプルプログラムです
M5stackのライブラリの使用を前提にしています
sipfのfwはversion４以上を前提にしています
find bme sensorと表示されたまま動かないときにはセンサーの接続を確認してください

使用した環境センサーについて
環境センサーにはBME280を使用しています https://www.switch-science.com/products/3374
使用ライブラリ https://github.com/Seeed-Studio/Grove_BME280 
アドレス 0x76 // https://github.com/Seeed-Studio/Grove_BME280/blob/master/Seeed_BME280.h

指定した時間で自動でデータを送信します
*/

/*
 動作確認環境
 ArduinoIDE 2.1.0
 board manager esp32 ver 2.0.8
 board : ESP32 Devmodule
*/

#include "esp_sipf_client.h"
#include "sipf_display.h"
// BME280を動かすためのライブラリ
#include "Seeed_BME280.h"
#include <Wire.h>


// 通信用の設定
#define NRF_RESET_PIN 5           // 通信ボードのリセットピン さくらのデフォルト設定
// 送信設定
#define TAG_ID_1 1                // 送信タグ
#define INT_TIME_SEC 30           // センサーを測定して送信する間隔（秒） 
const uint8_t tag_id = TAG_ID_1;
const int nrf_reset_pin = NRF_RESET_PIN;

ComunicationModule sipfModule;    // 通信用のインスタンス
BME280 bme280;                    // センサーオブジェクト

// 測定画面の作成
void CreateMeasurementScreen();
// センサーの値を測定
void MesureSensor(float *temp, float *humi, float *pres);
// 測定した値の表示
void DrawMesureValue(float temp, float humi, float pres);
// 測定値を元にpayloadを作成する関数
String MakePayload();
// JSON型のペイロードを作成する関数
String MakeJsonPayload(float temp, float humi, float pres);

// 画面の状態 0：初期化待ち、1：通常画面
volatile int  screen_status = 99; // 初期値
// タイマー用の変数と表示用の変数
long timer_mill = 0;
float temp = 99, humi = 99, pres = 0;     // 測定
float temp_old, humi_old, pres_old;

void setup() {

  Serial.begin(115200);
  delay(10);
  Serial.println("environ sensor start");

  ScreenCOnfig();           // 画面の基本設定

  // センサーを探す
  Serial.println("find bme sensor...");
  M5.Lcd.println("find bme sensor...");
  if(!bme280.init()){
    Serial.println("Device error!");
  }

  // タイマーリセット
  if(timer_mill == 0){timer_mill = millis();}
}

void loop(){

  sipfModule.loop();      // 通信モジュールの処理ループ

  MesureSensor(&temp, &humi, &pres);
  
  // 画面に表示
  if(abs(temp - temp_old) > 1 || abs(humi - humi_old) >1 ||abs( pres - pres_old) > 2){
    DrawMesureValue(temp, humi, pres);
  }

  // 時間毎に送信
  if( millis() - timer_mill> INT_TIME_SEC * 1000){
    Serial.printf("senddata t: %f, h: %f, p: %f\n", temp, humi, pres);
    sipfModule.SendPayload(MakeJsonPayload(temp, humi, pres).c_str());
    timer_mill = millis();
  }
}


// センサーの値を測定
void MesureSensor(float *temp, float *humi, float *pres){

  *temp = bme280.getTemperature();
  *pres = bme280.getPressure() / 100;  // hPaに変換
  *humi = bme280.getHumidity();
}

// 測定した値の表示画面の作成
void CreateMeasurementScreen(){
  
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.fillRect(0, 0, 320, 240, TFT_BLACK);
  M5.Lcd.fillRect(0, 0, 320, 40, TFT_BLACK);
  M5.Lcd.drawLine(0, 40, 320, 40, TFT_WHITE);
  M5.Lcd.drawString("temp", 10, 45, 4);
  M5.Lcd.drawString("c", 250, 45, 4);  
  M5.Lcd.drawLine(0, 140, 320, 140, TFT_WHITE);
  M5.Lcd.drawString("humi", 10, 145, 4);
  M5.Lcd.drawString("%RH", 250, 145, 4);
  M5.Lcd.drawLine(0, 175, 320, 175, TFT_WHITE);
  M5.Lcd.drawString("press", 10, 180, 4);
  M5.Lcd.drawString("hPa", 250, 180, 4);
  M5.Lcd.drawLine(0, 210, 320, 210, TFT_WHITE);
  M5.Lcd.setTextColor(TFT_WHITE  );
  M5.Lcd.drawString("environ monitoring", 40, 5, 4);
}

// 測定した値の表示
void DrawMesureValue(float temp, float humi, float pres){

  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  M5.Lcd.fillRect(80, 40, 160, 140, TFT_BLACK);   //表示部分のクリア
  M5.Lcd.fillRect(80, 140, 160, 175, TFT_BLACK);
  M5.Lcd.fillRect(80, 175, 160, 180, TFT_BLACK);
  char buf[8]; 
  dtostrf(temp, 5, 1, buf);
  M5.Lcd.drawString(buf, 85, 45, 6);
  dtostrf(humi, 3, 1, buf);
  M5.Lcd.drawString(buf, 100, 145, 4);
  dtostrf(pres, 3, 1, buf);
  M5.Lcd.drawString(buf, 100, 180, 4);
  M5.Lcd.drawLine(0, 40, 320, 40, TFT_WHITE);
  M5.Lcd.drawLine(0, 140, 320, 140, TFT_WHITE);
  M5.Lcd.drawLine(0, 175, 320, 175, TFT_WHITE);
  M5.Lcd.drawLine(0, 210, 320, 210, TFT_WHITE);
  temp_old = temp;
  humi_old = humi;
  pres_old = pres;
}

// payloadをJSONとして作成
String MakeJsonPayload(float temp, float humi, float pres){

  String payload;
  char buff[1024];  // 余裕を持ったサイズにしている
  sprintf(buff,"{\"t\":%f, \"h\":%f, \"p\": %f}", temp, humi, pres); 
  payload = buff;
  return payload;
}


// モジュールの初期化待ちの際に実行したい処理はこちらに書いてください
void InitWaitFunction(){

  // モジュールの初期化の際に表示される画面です
  if(screen_status != 0){
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0, 2);
    M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK); 
    M5.Lcd.println("reset the sipf module");
    M5.Lcd.println("module booting ....");
    screen_status = 0;
  }  
}

// モジュールがレディ状態の時に実行したい処理はこちらに書いてください
void ModuleReadyFunction() {

  // 測定値の表示を行う
  if(screen_status != 1){  
    CreateMeasurementScreen();
    DrawMesureValue(temp, humi, pres);
    screen_status = 1;
  }
}


