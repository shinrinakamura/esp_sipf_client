# Esp_sipf_client

このスケッチはM5Stackからさくらのモノプラットフォームを利用するためのサンプルプログラムです。
M5Stack COREとさくらのモノプラットフォーム標準ファームウェアを書き込んだ M5Stack対応LTEモジュール cat.M1 nRF9160（SCO-M5SNRF9160） との組み合わせで動作します。
導入に関しては[ご利用の流れ](https://manual.sakura.ad.jp/cloud/iotpf-beta/getting-started/gs-scom5snrf9160-beta.html)も参考にしてください。

現在は送信部分しか実装されていません。
使用するにはarduinoIDE、ESP32のボードマネージャー、M5stackのライブラリが必要になります。<br>
さくらインターネットが提供するM5stack用の開発ボードとM5stack basicで使用することを前提にしています。
通信モジュールのFWはver4以上を前提にしています。
<br>
zipファイルとしてダウンロードして、Arduinoからライブラリのインストールからzipファイルとして読み込んでください。<br>
<br>
Arduinoから、ESP32のボードを選択してM5stackに書き込みを行います。<br>
<br>
sample_environ_sensorを使用する際には、BME280センサーが必要になります。<br>
[https://www.switch-science.com/products/3374](https://www.switch-science.com/products/3374)<br>
使用する際にはこちらのライブラリをインストールしておく必要があります。<br>
[https://github.com/Seeed-Studio/Grove_BME280](https://github.com/Seeed-Studio/Grove_BME280)<br>
<br>
動作確認環境
M5stack basic<br>
ArduinoIDE 2.1.0<br>
board manager esp32 ver 2.0.8<br>
board : ESP32 Devmodule<br>
baud rate : 115200<br>
<br>
受信動作、OTIDの取り出しについては未実装。<br>
順次実装していく予定である。
