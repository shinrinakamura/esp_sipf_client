// ライブラリ公開用のサンプルプログラムです
// 通信モジュールのfwはversion４以上を前提にしています

// ボタンAを押すとカウンター１が１ずつ、ボタンBを押すとカウンター２が１０ずつカウントアップします
// ボタンCを押すとカウンターをクリアします
// jsonで送信するようになっています。key b1,b2,b3
// カウンターは100までしか上がらないようになっています

/*
 動作確認環境
 ArduinoIDE 2.1.0
 board manager esp32 ver 2.0.8
 board : ESP32 Devmodule
*/

#include "esp_sipf_client.h"
#include "sipf_display.h"

// 通信用の設定
//#define NRF_RESET_PIN 2         // 通信ボードのリセットピン 自社開発基板にあわせてある
#define NRF_RESET_PIN 5           // 通信ボードのリセットピン さくらのデフォルト設定
// 送信設定
#define TAG_ID_1 1                // 送信タグ
const uint8_t tag_id = TAG_ID_1;
const int nrf_reset_pin = NRF_RESET_PIN;

ComunicationModule sipfModule;    // 通信用のインスタンス

// M5stackのボタンの設定
#define BTN_A 39
#define BTN_B 38
#define BTN_C 37

// ボタンのセット
void BtnInit();
// ボタンAを押下した時の動作
IRAM_ATTR  void BtnPush_A();
// ボタンBを押下した時の動作
IRAM_ATTR void BtnPush_B();
// ボタンCを押下した時の動作
IRAM_ATTR void BtnPush_C();
// 測定値を元にpayloadを作成する関数
String MakePayload();
// JSON型のペイロードを作成する関数
String MakeJsonPayload(int value1, int value2);

// 画面の状態 0：初期化待ち、1：通常画面
volatile int  screen_status = 99; // 初期値
// カウンター用の変数
int counter1 = 0;
int counter2 = 0;


void setup(void) {
  
  Serial.begin(115200);
  delay(10);
  Serial.println("communication counter start");
  BtnInit();                // ボタンのセット
  ScreenCOnfig();           // 画面の基本設定
}

void loop() {
  
  sipfModule.loop();        // 通信モジュールの処理ループ
  delay(1);                 // このdelayを入れないとボタンの動作が不安定になることがある
}


// ボタンの設定-------------------------------------------------
void BtnInit(){
  // pinの設定
   pinMode(BTN_A, INPUT);
   pinMode(BTN_B, INPUT);
   pinMode(BTN_C, INPUT);
   // ボタンの割り込み設定
   attachInterrupt(BTN_A, BtnPush_A, FALLING);
   attachInterrupt(BTN_B, BtnPush_B, FALLING);
   attachInterrupt(BTN_C, BtnPush_C, FALLING);
}


// ボタンAを押下した時の動作
IRAM_ATTR  void BtnPush_A(){
  // カウンター1のカウントアップ
  Serial.println("button A push");
  counter1 = counter1 +1;
  if(counter1 > 100){
    counter1 = 100;
  }
  M5.Lcd.drawNumber(counter1, 37, 175);
  // payloadを作って送信
  String payload = MakeJsonPayload(counter1, counter2);
  sipfModule.SendPayload(payload.c_str());
}

// ボタンBを押下した時の動作
IRAM_ATTR void BtnPush_B(){
  // カウンタ－２のカウントアップ
  Serial.println("button B push");
  counter2 = counter2 +10;
  if(counter2 > 100){
    counter2 = 100;
  }
  M5.Lcd.drawNumber(counter2, 133, 175);
  // payloadを作って送信
  String payload = MakeJsonPayload(counter1, counter2);
  sipfModule.SendPayload(payload.c_str());
}

// ボタンCを押下した時の動作
IRAM_ATTR void BtnPush_C(){
  // カウンターのクリア
  Serial.println("button C push");
  counter1 = 0;                                     // カウンターのリセット
  counter2 = 0;
  M5.Lcd.fillRect(35, 163, 60, 60, TFT_WHITE);      // counter1
  M5.Lcd.fillRect(130, 163, 60, 60, TFT_WHITE);     // counter2
  M5.Lcd.drawNumber(counter1, 37, 175);
  M5.Lcd.drawNumber(counter2, 133, 175);
  // payloadを作って送信
  String payload = MakeJsonPayload(counter1, counter2);
  sipfModule.SendPayload(payload.c_str());
}

// payload用の文字列の作成
String MakePayload(){
  String payload;
  payload = "";
  return payload;
}

// 送信するJSONの作成
String MakeJsonPayload(int val_1, int val_2){
  String payload;
  char buff[1024];  // 余裕を持ったサイズにしている
  sprintf(buff,"{\"b1\":%d,\"b2\":%d}", val_1, val_2); 
  payload = buff;
  return payload;
}


// モジュールの初期化待ちの際に実行する関数
void InitWaitFunction(){
  // モジュールの初期化待ちの際に実行したい処理はここに書いてください
  if(screen_status != 0){     // モジュールの初期化の際に表示される画面です
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 0, 2);
    M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK); 
    M5.Lcd.println("reset the sipf module");
    M5.Lcd.println("module booting ....");
    screen_status = 0;
  }
}

// モジュールがレディ状態の時に実効される関数
void ModuleReadyFunction() {
  // モジュールがレディ状態の時に表示される関数についてはここに書いてください
  if(screen_status != 1){     // カウンターの画面の表示
    IndicateCounterScreen(counter1, counter2);  // カウンター用の画面の作成  
    screen_status = 1;
  }
}
