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

// SDカードアクセス用
File myFile;							
const int maxlen = 64;					// 一度にファイルから読むbyte数
char buffer[maxlen+1];					// サイズは終端文字を残すために1バイト余分に取る

// Ethernetアクセス用
byte mac[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// EthernetシールドのMACアドレス(→TBD)
};
IPAddress ip(0, 0, 0, 0);				// EthernetシールドのローカルIPアドレス(→TBD)
EthernetServer server(80);				// ポート80番(HTTP)

// DDNSサービスアクセス用
unsigned long lastCheckedTime = 0;				// 前回確認時
const unsigned long ddnsInterval = 360000;	    // DDNS接続周期(1時間)
void ddns(void);								// DDNS確認リクエスト送信


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
	; // wait for serial port to connect. Needed for Leonardo only
  }

  //Give time to initilize:
  delay(1000);

  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
	  Serial.println("failed!");
	  return;
  }
  Serial.println("done.");


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  //Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
	//Serial.println("new client");

	while (client.connected()) {
	  if (client.available()) {
		char c = client.read();
		//Serial.write(c);	// リクエストの内容

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
			  //Serial.println("Index.htm:");

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

			  // ファイルオープンに失敗したことを通知:
			  //Serial.println("error opening Index.htm");

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
  if ( (abs(millis() - lastCheckedTime) > ddnsInterval) )
  {
	  Serial.println();
	  lastCheckedTime = millis();
	  ddns();
  }
  else
  {
	  Serial.print(ddnsInterval);
	  Serial.print(" ");
	  Serial.print(abs(millis() - lastCheckedTime));
	  Serial.println();
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

	if (ddnsclient.connect("dynupdate.no-ip.com", 80)) {
		//Serial.println("Connected to noip");

		ddnsclient.println("GET /nic/update?hostname=zkure.ddns.net HTTP/1.0");
		ddnsclient.println("Host: dynupdate.no-ip.com");
		ddnsclient.println("Authorization: Basic bWFpbEBleGFtcGxlLmNvbTpwYXNzd29yZA==");	//BASE64に変換 mail@example.com:password→bWFpbEBleGFtcGxlLmNvbTpwYXNzd29yZA==
		ddnsclient.println("User-Agent: username Arduino Client/0.0 mail@example.com");
		ddnsclient.println();

		//Wait for response
		delay(5000);
		//Serial.print("Characters available: ");
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
