
const String ddnsMySiteName = "example.com";
const String ddnsHostName	= "dynupdate.no-ip.com";
const String ddnsAuthCode = "bWFpbEBleGFtcGxlLmNvbTpwYXNzd29yZA==";	//BASE64に変換 mail@example.com:password→bWFpbEBleGFtcGxlLmNvbTpwYXNzd29yZA==
const String ddnsMailAddress	= "mail@example.com";

// Ethernetアクセス用
byte mac[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// EthernetシールドのMACアドレス
};
IPAddress ip(192, 168, 11, 99);			// EthernetシールドのローカルIPアドレス

