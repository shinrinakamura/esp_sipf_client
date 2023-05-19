// 開発メモ
// タイムアウトの回数を数えていても良いかもしれない
// OTIDも取り出せるようにしておく必要がある

#include "esp_sipf_client.h"

/*コンストラクタ*/
// 第一引数：リセットピン
// 第二引数：シリアルクラス
ComunicationModule::ComunicationModule(int resetpin, HardwareSerial *serial){
  InitModule(resetpin, serial);
}

// デフォルトコンストラクタ
// さくらの通信モジュールとM5stackの組合わせで使用するときのデフォルト設定になっている
ComunicationModule::ComunicationModule(){
  InitModule(nrf_reset_pin, &Serial2);    
}

// main loopにおいておく必要があります
void ComunicationModule::loop(){
  
  switch (_status)                // ステートマシンの記述
  {
    // モジュールが準備できていない
    //    要因：起動時、モジュールから応答がない際のリセット 
    case  not_ready:              
      _status = module_reset;    // リセット状態に遷移する
      break;

    // モジュールのリセット中
    case  module_reset:           
      ResetModule();              // モジュールをリセットする
      _status = init_wait;        // モジュールの初期化の待機に移行する
      break;
    
    // モジュールの初期化待機中
    case  init_wait:  
      _init_wait_status = WaitForInitResponce();
      if(_init_wait_status == CONTINUATION){  
        InitWaitFunction();       // 待機状態を継続する
      }else{
        // 失敗した際には再度リセットする
        if(_init_wait_status == SUCCESS){
          _init_failure_count = 0; 
          _connection_status = true;
          _status = module_ready;   // コマンド受付可能な状態に遷移する
        }else{
          // 指定した回数連続で失敗したら一旦あきらめる（指定した時間毎に再接続に挑戦）
          if(_init_failure_count >= INIT_FAILURE_COUNT){
            _init_failure_count = 0; 
            _connection_status = false;
            Serial.printf("give up connect\ntransfer module_ready\n");
            _status = module_ready;
          }else{
            // 接続に失敗した場合は上限回数まで再チャレンジ
            _init_failure_count += 1;  
            _connection_status = false;
            Serial.printf("\nfailed count %d and reset\n", _init_failure_count);  // テスト表示
            _status = module_reset;
          } 
        }
      }
      break;

    // モジュールの準備ができている（送信可能）
    case  module_ready:           
      // ready状態の維持（SendPaylod関数が実行された際にmodule_sendに遷移する）
      ModuleReadyFunction();    // モジュールレディ状態の時に実行される関数
      break;

    // 送信中
    case  module_send:         
      SendPayload();            // payloadを送信
      _status = wait_response;  // モジュールからのレスポンスを待ちに遷移する
      break;

    // nRFからの送信レスポンスを待つ
    case  wait_response:  
      _cmd_wait_status = WaitForCommandResponce() ;
      if(_cmd_wait_status == CONTINUATION){
        // 待機状態を継続する
      }else if(_cmd_wait_status == SUCCESS){
        // 送信に成功したらコマンド受付可能な状態に遷移する
        // _send_failure_cont = 0;
        _status = module_ready;
      }else{
        // 失敗した時はモジュールをリセットする
        Serial.printf("send failure module reset ");  // テスト表示
        _status = module_reset;
      }
      break;

  default:
    break;

  }
} 

// ステータスをシリアルに表示（テスト用関数）
void ComunicationModule::IndicateStatus(){  
  switch (_status)
  {
  case  not_ready:      // モジュールが準備できていない
    Serial.println("status is not ready");
    break;
  case  module_reset:   // モジュールのリセット中
    Serial.println("status is module_reset");
    break;
  case  init_wait:      // モジュールの初期化待機中
    Serial.println("status is init_wait");
    break;
  case  module_ready:   // モジュールの準備ができている（送信可能）
    Serial.println("status is module_ready");
    break;
  case  module_send:    // 送信中
    Serial.println("status is module_send");
    break;
  case  wait_response:    // 送信中
    Serial.println("status is wait_response");
    break;
  }
}

// 現在のモジュールの状態を文字列で返す
String ComunicationModule::GetStatusString(){
  String status_st;
  switch (_status)  
  {
  case  not_ready:      // モジュールが準備できていない
    Serial.println("status is not ready");
    status_st = "ready";
    break;
  case  module_reset:   // モジュールのリセット中
    Serial.println("status is module_reset");
    status_st = "module_reset";
    break;
  case  init_wait:      // モジュールの初期化待機中
    Serial.println("status is init_wait");
    status_st = "init_wait";
    break;
  case  module_ready:   // モジュールの準備ができている（送信可能）
    Serial.println("status is module_ready");
    status_st = "module_ready";
    break;
  case  module_send:    // 送信中
    Serial.println("status is module_send");
    status_st = "module_send";
    break;
  case  wait_response:    // 送信中
    Serial.println("status is wait_response");
    status_st = "wait_response";
    break;
  }
  return status_st;
}    


// ペイロードの送信
// 送信出来なかった時にfalseを返す
bool ComunicationModule::SendPayload(const char *payload){
  // ステータス確認（テスト）
  IndicateStatus();
  // モジュールの状態を確認
  if(_status == module_ready){
    _payload_buff = "";      //バッファのクリア
    _payload_buff = payload;
    _cmd_buff_len = _payload_buff.length();
    Serial.print("payload length : ");  // 長さを表示して確認する（テスト）
    Serial.println(_cmd_buff_len);
    
    if(_cmd_buff_len >= MAX_PAYLOAD_SIZE){  // バッファがオーバーフローの確認
      Serial.println("payload size too long");
      return false;
    }
    if(_connection_status == true){     // 通信が確立していればpayloadを送信する
      _status = module_send;
    }
    return true;
  }else{
    return false;
  }
  return false;
} 

// リセットピンとハードウェアシリアルを指定する
// 第一引数：リセットピン
// 第二引数：シリアルクラス
void ComunicationModule::InitModule(int resetpin, HardwareSerial *serial){
  _reest_pin = resetpin;    // 通信ボードをリセットするピン
  _serial = serial;
  _serial->begin(NRF_BAUD_RATE);
  _status = not_ready;      // モジュールが準備できていない
  // pinのセット
  digitalWrite(_reest_pin, LOW);
  pinMode(_reest_pin, OUTPUT);
  _start_time = 0;          // 無効値として0を代入する
}

// 実際にpayloadを送信する関数
void ComunicationModule::SendPayload(){
  _status = wait_response;    // wait状態に遷移
  // テスト用の表示
  Serial.print("ComunicationModule::SendPayload()\n ");
  Serial.println(_payload_buff.c_str());
  //コマンドを作成する
  int len;
  len = sprintf(_make_cmd_buff, "$$TX %02X %02X ",  tag_id, (uint8_t)PAYLOAD_TYPE);
  // ここの方法はさくらのサンプルを参考にしている
  for (int i = 0; i < _cmd_buff_len; i++) {
    len += sprintf(&_make_cmd_buff[len], "%02X", _payload_buff[i]);
  }
  len += sprintf(&_make_cmd_buff[len], "\r\n");
  Serial.println();       // テスト用に表示を整えている
  Serial.println(&_make_cmd_buff[len]);
  _serial->write((uint8_t*)_make_cmd_buff, len);      // 送信処理

  _make_cmd_buff[0] = '\0';
}

// 通信モジュールのリセットを行なう
void ComunicationModule::ResetModule(){
  Serial.println("");
  Serial.println("ComunicationModule::ResetModule()");
  digitalWrite(_reest_pin, HIGH);
  delay(RESET_WAIT);
  digitalWrite(_reest_pin, LOW);
}

// リセット後の初期化を待つ
// 返り値
// true 待機状態
// false 待機終了
#define INIT_READY_STRING "+++ Ready +++"
int ComunicationModule::WaitForInitResponce(){
  bool ret;
  int b;
  long responseTime = 0;  // レスポンス時間の計測（テスト）
  ret = CONTINUATION; // テスト値
  if(_start_time == 0){
    // 最初に入ってきたとき
    _start_time = millis();    // 時刻を記録
    // 受信バッファをクリア
    _recv_buff = "";
  }
  // タイムアウトの判定を行なう
  if( _start_time + INIT_WAIT_TIME_SEC * 1000  < millis()){
    // レスポンス時間の表示（テスト）
    responseTime = millis() - _start_time;
    Serial.println("init timeout");
    Serial.printf("init time out elapsed time : %ld ms\n", responseTime);
    _start_time = 0;
    return TIMEOUT;
  }
  // 1行分切り出す
  if(_serial->available()){
    b = _serial->read();
    // 改行コードを検出した場合
    if (b == '\r'){
      // テストで表示する
      Serial.println(_recv_buff);
      if(_recv_buff.compareTo(INIT_READY_STRING) == 0 ){
        // レスポンス時間の表示（テスト）
        responseTime = millis() - _start_time;
        Serial.printf("init elapsed time : %ld ms\n", responseTime);
        Serial.print("init done");
        _recv_buff = "";   // バッファクリア
        ret = SUCCESS;
      }
      
    }else if( b == '\n'){
    // 何もしない
    _recv_buff = "";   // 念のためバッファクリア
    }else{
    // 改行コード以外は文字列の連結
    _recv_buff = _recv_buff + (char) b;
    }
  }
  // ステートの遷移した時刻をクリア（テストコードも兼ねる）
  if (ret == SUCCESS){
    // Serial.println("ComunicationModule::WaitForInitResponce is false (wait state is end)");
    _start_time = 0;
  }else{
    // Serial.println("ComunicationModule::WaitForInitResponce is true (waiting)");
  }
  return ret;
}

// コマンドのレスポンスを待ち受ける関数
// 返り値：0 -> 待ち受け継続、1 -> 成功、2 -> 失敗、3 -> タイムアウト
int ComunicationModule::WaitForCommandResponce(){
  int ret;
  int b;
  ret = CONTINUATION; // 待ち受けの状態
  long responseTime = 0; // レスポンスを計測するための変数（テスト用）
  if(_start_time == 0){
    // 最初に入ってきたとき
    _start_time = millis();    // 時刻を記録
    // 受信バッファをクリア
    _recv_buff = "";
  }
  // タイムアウトの判定を行なう
  if( _start_time + CMD_WAIT_TIME_SEC * 1000  < millis()){
    Serial.println("response time out");
    // レスポンス時間の表示（テスト用）
    responseTime = millis() - _start_time;
    Serial.printf("elapsed time : %ld ms\n", responseTime);
    _start_time = 0;    // 時間のリセット
    return TIMEOUT;       // タイムアウトの処理を改めて書き直す
  }
  // 1行分切り出す
  if(_serial->available()){
    b = _serial->read();
    // 改行コードを検出した場合
    if (b == '\r'){
      Serial.println(_recv_buff);
      if(_recv_buff.compareTo("OK") == 0 ){
        Serial.println("send OK");
        // レスポンス時間の表示（テスト）
        responseTime = millis() - _start_time;
        Serial.printf("elapsed time : %ld ms\n", responseTime);
        _start_time = 0;    // 時間のリセット
        ret = SUCCESS;
      }else if(_recv_buff.compareTo("NG") == 0){
        Serial.println("send NG");
        // レスポンス時間の表示（テスト）
        responseTime = millis() - _start_time;
        Serial.printf("elapsed time : %ld ms\n", responseTime);
        _start_time = 0;    // 時間のリセット
        _recv_buff = "";    // バッファクリア
        ret = FAILURE;
      }
    }else if( b == '\n'){
    // 何もしない
    _recv_buff = "";   // 念のためバッファクリア
    }else{
    // 改行コード以外は文字列の連結
    _recv_buff = _recv_buff + (char) b;
    }  
  }
  return ret;
}                

// 接続しているかどうか確認する処理（一定時間毎に確認するようにすると良い）
// 通信が確立していなければモジュールをリセットして接続に挑戦
void ComunicationModule::CommunicationConfirmation(){
  if(_connection_status == false){
    _status = module_reset;    // リセット状態に遷移する
  }
}