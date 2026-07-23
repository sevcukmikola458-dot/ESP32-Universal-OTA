#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <Update.h>

WebServer server(80);

const char* apName = "ESP32-Universal-OTA";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>ESP32 Universal OTA</title>
<style>
body{font-family:Arial;text-align:center;background:#eee;margin-top:40px}
.box{background:white;padding:25px;border-radius:10px;display:inline-block}
button{padding:12px 25px;font-size:18px}
</style>
</head>
<body>
<div class="box">
<h2>ESP32 Universal OTA</h2>
<form method="POST" action="/update" enctype="multipart/form-data">
<input type="file" name="update">
<br><br>
<button>Прошити BIN</button>
</form>
</div>
</body>
</html>
)rawliteral";


void setup() {

  Serial.begin(115200);

  WiFiManager wm;

  wm.setConfigPortalTimeout(180);

  if (!wm.autoConnect(apName)) {
    Serial.println("WiFi failed");
    ESP.restart();
  }

  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());


  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });


  server.on("/update", HTTP_POST, []() {

    server.send(200, "text/plain",
    Update.hasError() ? 
    "ERROR" : 
    "OK. Restarting");

    delay(1000);
    ESP.restart();

  }, []() {

    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {

      Serial.printf("Update: %s\n",
      upload.filename.c_str());

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }

    } 
    else if (upload.status == UPLOAD_FILE_WRITE) {

      if (Update.write(
        upload.buf,
        upload.currentSize) != upload.currentSize) {

        Update.printError(Serial);
      }

    } 
    else if (upload.status == UPLOAD_FILE_END) {

      if (Update.end(true)) {

        Serial.printf(
        "Success: %u bytes\n",
        upload.totalSize);

      } 
      else {
        Update.printError(Serial);
      }
    }
  });


  server.begin();

}


void loop() {

  server.handleClient();

}
