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

#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>
#include "WebConstants.h"

// Ethernetアクセス用
EthernetServer server(80);				// ポート80番(HTTP)

// SDカードアクセス用
File myFile;							
const int maxlen = 64;					// 一度にファイルから読むbyte数
char buffer[maxlen+1];					// サイズは終端文字を残すために1バイト余分に取る

// DDNSサービスアクセス用
unsigned long ddnsCheckedTime = 0;					// DDNS前回確認時
unsigned long ddnsCheckTimer = 0;					// DDNS確認タイマー
const unsigned long ddnsInterval = (1000*60*60);	// DDNS接続周期(1時間)
void ddns(void);									// DDNS確認リクエスト送信


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
	; // wait for serial port to connect. Needed for Leonardo only
  }

  //初期化のための待ち時間:
  delay(1000);

  Serial.print("SDカード初期化...");
  if (!SD.begin(4)) {
	  Serial.println("failed!");
	  return;
  }
  Serial.println("done.");


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  // サーバーのローカルIP
  Serial.println(Ethernet.localIP());
}


void loop() {
  // 受信待受
  EthernetClient client = server.available();
  if (client) {
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

		  // 読み出しモードで開く:
		  myFile = SD.open("Index.htm");	  // ファイル名は8.3形式
		  if (myFile) {
			  // ファイルから読み出すデータが無くなるまで読む:
			  while (myFile.available()) {				  				  
				  memset(buffer, 0x00, sizeof(buffer));

				  int length = myFile.available();
				  if (length > maxlen) {
					  length = maxlen;
				  }
				  myFile.read(buffer, length);          

				  client.print(buffer);        // 送信する
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
	//Serial.println("client disconnected");
  }


  // DDNS確認周期
  ddnsCheckTimer = millis() - ddnsCheckedTime;
  if ( (abs(ddnsCheckTimer) > ddnsInterval) )
  {
	  Serial.println();
	  ddnsCheckedTime = millis();
	  ddns();
  }


}

// DDNS確認リクエスト送信
void ddns() {
	Serial.println("DDNS...");
	//Give time to initilize:
	delay(1000);

	EthernetClient ddnsclient;

	// start the Ethernet
	Ethernet.begin(mac, ip);

	//Give time to initilize:
	delay(1000);

	if (ddnsclient.connect(ddnsHostName.c_str(), 80)) {
		//Serial.println("Connected to noip");
		String strbuf;

		strbuf = "GET /nic/update?hostname=" + ddnsMySiteName + String(" HTTP/1.0");
		ddnsclient.println(strbuf);

		strbuf = "Host: " + ddnsHostName;
		ddnsclient.println(strbuf);

		strbuf = "Authorization: Basic " + ddnsAuthCode;
		ddnsclient.println(strbuf);

		strbuf = "User-Agent: username Arduino Client/0.0 " + ddnsMailAddress;
		ddnsclient.println(strbuf);

		ddnsclient.println();

		//Wait for response
		delay(5000);
		// 応答内容を表示
		Serial.println(ddnsclient.available());

		while (ddnsclient.available() > 0)
		{
			char read_char = ddnsclient.read();
			Serial.print(read_char);
		}
	}
	else {
		// kf you didn't get a connection to the server:
		//Serial.println("connection failed");
	}

}
