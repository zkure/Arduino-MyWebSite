#define ArrayCnt(ary) (sizeof(ary)/sizeof(ary[0]))  // 配列要素数取得マクロ

const int ddnsPort = 21;							// ポート番号(FTP)
const String ddnsMySiteName = "example.com";		// 登録済みドメイン
const String ddnsHostName = "ftp.mydns.jp";			// DDNSホストサーバ
const String ddnsAuthUser = "username";				// DDNSログインユーザ名
const String ddnsAuthPass = "password";				// DDNSログインパスワード
const String ddnsRequestStrs[] = {					// リクエストメッセージ
	"USER " + ddnsAuthUser,
	"PASS " + ddnsAuthPass
};

const String ftpAuthUser = "username";  // FTPログインユーザ名
const String ftpAuthPass = "password";  // FTPログインパスワード

// Ethernetアクセス用
byte mac[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// EthernetシールドのMACアドレス
};
IPAddress ip(192, 168, 11, 99);			// EthernetシールドのローカルIPアドレス
