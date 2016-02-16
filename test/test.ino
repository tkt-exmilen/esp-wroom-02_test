#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>


// モード切り替えピン
const int MODE_PIN = 0; // GPIO0

// Wi-Fi設定保存ファイル
const char* settings = "/wifi_settings.txt";

// サーバモードでのパスワード
const String pass = "thereisnospoon";


ESP8266WebServer server(80);

/**
 * WiFi設定
 */
void handleRootGet() {
  String html = "";
  html += "<h1>WiFi Settings</h1>";
  html += "<form method='post'>";
  html += "  <input type='text' name='ssid' placeholder='ssid'><br>";
  html += "  <input type='text' name='pass' placeholder='pass'><br>";
  html += "  <input type='submit'><br>";
  html += "</form>";
  server.send(200, "text/html", html);
}

void handleRootPost() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  File f = SPIFFS.open(settings, "w");
  f.println(ssid);
  f.println(pass);
  f.close();

  String html = "";
  html += "<h1>WiFi Settings</h1>";
  html += ssid + "<br>";
  html += pass + "<br>";
  server.send(200, "text/html", html);
}

/**
 * HTTPリクエスト（GET）
 */
void http_get() {
  HTTPClient http;
  http.begin("http://weather.livedoor.com/forecast/webservice/json/v1?city=400040");
  int httpCode = http.GET();
  if(httpCode) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if(httpCode == 200) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %d\n", httpCode);
  }

  http.end();
  delay(5000);
}

/**
 * 初期化(クライアントモード)
 */
void setup_client() {

  File f = SPIFFS.open(settings, "r");
  String ssid = f.readStringUntil('\n');
  String pass = f.readStringUntil('\n');
  f.close();

  ssid.trim();
  pass.trim();

  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);

  WiFi.begin(ssid.c_str(), pass.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  http_get();
}

/**
 * 初期化(サーバモード)
 */
void setup_server() {
  byte mac[6];
  WiFi.macAddress(mac);
  String ssid = "";
  for (int i = 0; i < 6; i++) {
    ssid += String(mac[i], HEX);
  }
  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + pass);

  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid.c_str(), pass.c_str());

  server.on("/", HTTP_GET, handleRootGet);
  server.on("/", HTTP_POST, handleRootPost);
  server.begin();
  Serial.println("HTTP server started.");
}

/**
 * 初期化
 */
void setup() {
  Serial.begin(115200);
  Serial.println();

  // 1秒以内にMODEを切り替える
  //  0 : Server
  //  1 : Client
  delay(1000);

  // ファイルシステム初期化
  SPIFFS.begin();

  pinMode(MODE_PIN, INPUT);
  if (digitalRead(MODE_PIN) == 0) {
    // サーバモード初期化
    setup_server();
  } else {
    // クライアントモード初期化
    setup_client();
  }
}

void loop() {
  server.handleClient();
}

