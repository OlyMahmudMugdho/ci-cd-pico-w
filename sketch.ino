#include <WiFi.h>

const char* ssid = "Logicon-GUEST";
const char* password = "";

void setup() {
  Serial1.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }

  Serial1.println("\nWiFi connected!");
  Serial1.println(WiFi.localIP());
}

void loop() {
}
