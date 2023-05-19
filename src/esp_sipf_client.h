// 未実装の機能
// 接続ステータスを返す関数（接続しているか、電波強度等）
// 受信機能

#ifndef __ESP_SIPF_CLIENT_H__
#define __ESP_SIPF_CLIENT_H__

#include <Arduino.h>

#define NRF_BAUD_RATE 115200        // マイコンと通信モジュールのボーレート  https://github.com/sakura-internet/sipf-std-client_nrf9160/wiki
#define RESET_WAIT  10              // リセット時間（さくらのサンプルコードより流用）
#define INIT_WAIT_TIME_SEC 120      // 初期化の待ち時間(sipf clientのタイムアウト時間が60秒というところから決定)(要確認)
#define CMD_WAIT_TIME_SEC 10        // コマンドのレスポンスの待ち時間
#define MAX_PAYLOAD_SIZE 1024       // コマンド全体で1024byteになるのでユーザーが使用できるのは1020byte
#define INIT_FAILURE_COUNT 2        // 初期化の失敗の上限（接続に失敗したら送信しないモードに入る）
// #define FAILURE_COUNT 2             // あきらめて送信失敗にする回数
// #define THROUGH_COUNT 3             // この回数送信を連続失敗したらリセットをかける

// レスポンス：0 -> 待ち受け継続、1 -> 成功、2 -> 失敗、3 -> タイムアウト
#define CONTINUATION 0              // 待ち受け継続
#define SUCCESS 1                   // 成功
#define FAILURE 2                   // 失敗
#define TIMEOUT 3                   // タイムアウト

// 使用するペイロードタイプを選択します
#define PAYLOAD_TYPE 0x20           // str.utf8	文字列

// モジュールの状態
enum ModuleStatus{
  not_ready,      // モジュールが準備できていない
  module_reset,   // モジュールのリセット中
  init_wait,      // モジュールの初期化待機中
  module_ready,   // モジュールの準備ができている（送信可能）
  module_send,    // 送信中
  wait_response,  // コマンドのレスポンスを待つ
};

class ComunicationModule{
  public:   
    // コンストラクタ
    ComunicationModule();                       // 標準のコンストラクタ（デフォルト設定）
    ComunicationModule(int resetpin, HardwareSerial *serial); // 引数：リセットピン、モジュールと通信するシリアルポート

    void loop();                               // メインループの中においておく必要がある
    /* 通信できる状態でないとこの関数は失敗する */
    bool SendPayload(const char *payload);     // ペイロードを送信 ビジー状態の時に失敗とする
    void CommunicationConfirmation();          // 通信が確立しているか確認する（一定周期で呼び出す必要がある）
     
    // マクロ定義で指定されるようにしておく debugmodeなど 残作業 
    // テスト用関数
    void IndicateStatus();
    String GetStatusString();           // 現在のモジュールの状態を文字列で返す　未試験
    String GetOtid();
    // OTIDは表示できるようにしておく必要がありそう（一応文字列で取得できるようにしておく）

  private:
    void InitModule(int resetpin, HardwareSerial *serial); // 引数：リセットピン、モジュールと通信するシリアルポート
    void SendPayload();                 // 実際のpayloadの送信処理
    void ResetModule();                 // 通信モジュールのリセット（トラブルが起きたときに必要になる）
    int WaitForInitResponce();          // 通信モジュールのリセットのレスポンスを待つ
    int WaitForCommandResponce();       // 送信コマンドのレスポンスを待つ
    bool ResponseConfig();              // 設定を受信する（未実装）
    
    int _reest_pin;                     // 通信ボードをリセットするピン
    String _payload_buff;               // 送信バッファ
    String _recv_buff;                   // 受信バッファ
    String _otid_buff;
    char _make_cmd_buff[1024];          //  コマンドを作成する時のバッファ
    unsigned int _cmd_buff_len;         //  バッファの長さ 
    uint32_t _start_time;               // ステートが遷移した時刻
    volatile ModuleStatus _status;      // ステートマシンの状態
    HardwareSerial *_serial;            // モジュールとの通信用のシリアルポート
    bool _connection_status = true;     // 通信が接続しているかどうか  
    volatile int _init_wait_status = 0; // 初期化待ちの状態
    volatile int _cmd_wait_status = 0;  // コマンドのレスポンスの待ち状態
    int _init_failure_count = 0;        // 初期化に失敗した回数をカウント
};

extern const uint8_t tag_id;            // tagID
extern const int nrf_reset_pin;         // nRFモジュールのリセットピン
void InitWaitFunction();                // モジュールの初期化待ちの時に実効される関数
void ModuleReadyFunction();
#endif