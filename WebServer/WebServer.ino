/*
  簡易Webサーバ
  Arduino Uno+Ethernetシールドで実装

 * Ethernetでpin10,11,12,13を使う

 * SDカードの為にSPIバスと以下のピンを使う:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 * 概要
 ・HTTPリクエストに対してSDカードにあるIndex.htmを返す。
 ・定期的にDDNSサービスへリクエストを送信する。

 */

#include <MsTimer2.h>
#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include "WebConstants.h"

// Ethernetアクセス用
EthernetServer server(80);  // ポート80番(HTTP)

// SDカードアクセス用
File myFile;
const int maxlen = 64;  // 一度にファイルから読むbyte数
char buffer[maxlen+1];  // サイズは終端文字を残すために1バイト余分に取る

// DDNSサービスアクセス用
unsigned long ddnsCheckCount = 0;   // DDNS確認タイマー
const unsigned long ddnsIntervalNum  = 3600;   // DDNS接続周期
const unsigned long TimerInterval = 1000;   // 周期処理周期(msec) 

void ddns(void);    // DDNS確認リクエスト送信
void timerproc(void);   // 周期処理

void setup() {
  // シリアルポートオープン:
  Serial.begin(9600);
  while (!Serial) {
    ; // シリアルポートオープン待ち(Leonardoボード使用の場合)
  }

  //初期化待ち:
  delay(1000);

  Serial.print("SD Card Initialize...");
  if (!SD.begin(4)) {
      Serial.println("failed!");
      return;
  }
  Serial.println("done.");
  
  // Ethernet接続とサーバー開始:
  Ethernet.begin(mac, ip);
  server.begin();
  // サーバーのローカルIP
  Serial.println(Ethernet.localIP());
  
  // DDNS確認リクエスト送信
  ddns();

  // 周期処理タイマ初期化
  MsTimer2::set(TimerInterval, timerproc);
  MsTimer2::start();

  // LEDピン初期化
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // 受信待受
  EthernetClient client = server.available();
  if (client) {
    Serial.println("client connecting...");
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);	// リクエストの内容

        // リクエストの終端(改行文字)
        if (c == '\n') {
          // ここからHTTPレスポンスヘッダ
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // この応答の完了後に接続を閉じる
          client.println();
          // HTTPレスポンスヘッダここまで
          
          // ここからWebページ部分
          Serial.println("opening Index.htm");
          // 読み出しモードで開く:
          myFile = SD.open("Index.htm");    // ファイル名は8.3形式
          if (myFile) {
              // ファイルから読み出すデータが無くなるまで読む:
              while (myFile.available()) {
                  memset(buffer, 0x00, sizeof(buffer));

                  int length = myFile.available();
                  if (length > maxlen) {
                      length = maxlen;
                  }
                  myFile.read(buffer, length);          

                  client.print(buffer); // 送信する
              }
              // ファイルを閉じる:
              myFile.close();
          }
          else {
              // ファイルオープンに失敗
              Serial.println("error opening Index.htm");
          }
          break;
        }
      }
    }
    // ブラウザにデータ受信する時間を与える
    delay(1);
    // 接続を閉じる:
    client.stop();
    Serial.println("client disconnected");
  }

  // DDNS確認周期
  if ((ddnsCheckCount > ddnsIntervalNum))
  {
      ddns();
      ddnsCheckCount = 0;
  }
}

// 周期処理
void timerproc() {
    // DDNS確認周期   
    ddnsCheckCount++;
    // 動作確認
    digitalWrite(LED_BUILTIN, (ddnsCheckCount%2));
}

// DDNS確認リクエスト送信
void ddns() {
    EthernetClient ddnsclient;

    Serial.println("DDNS...");
    
    // DDNSサービス接続
    Serial.println(ddnsHostName.c_str());
    if (ddnsclient.connect(ddnsHostName.c_str(), ddnsPort)) {
        Serial.println("Connecting...");
        String strbuf;

        // DDNSリクエスト送信
        for (int i=0; i<ArrayCnt(ddnsRequestStrs); i++)
        {
            strbuf = ddnsRequestStrs[i];
            ddnsclient.println(strbuf);
            Serial.println(strbuf);
        }
        ddnsclient.println();
        
        // 応答待ち
        delay(5000);
        // 応答内容を表示
        while (ddnsclient.available() > 0)
        {
            char read_char = ddnsclient.read();
            Serial.print(read_char);
        }
        Serial.println("DDNS done.");
    }
    else {
        // DDNSサーバー接続失敗:
        Serial.println("DDNS failed!");
    }

}
