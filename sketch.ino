
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <MQTTPubSubClient.h>

// Wi-Fi credentials
const char* WIFI_SSID = "Logicon-GUEST";
const char* WIFI_PASSWORD = "";

// HiveMQ public WebSocket broker
const char* MQTT_HOST = "broker.hivemq.com";
const uint16_t MQTT_PORT = 8000;
const char* MQTT_PATH = "/mqtt";

// Choose a unique topic prefix for your device
const char* TOPIC_PREFIX = "pico-w-demo-74291";

String publishTopic;
String subscribeTopic;

WebSocketsClient client;
MQTTPubSubClient mqtt;

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 5000;

bool connectWiFi() {
  Serial.print("Connecting to Wi-Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWi-Fi connection failed.");
    return false;
  }

  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\nPico W MQTT over WebSockets");
  Serial.println("Broker: broker.hivemq.com:8000");

  publishTopic = String(TOPIC_PREFIX) + "/data";
  subscribeTopic = String(TOPIC_PREFIX) + "/command";

  if (!connectWiFi()) {
    return;
  }

  // Establish the WebSocket transport.
  client.begin(MQTT_HOST, MQTT_PORT, MQTT_PATH);
  client.setReconnectInterval(5000);

  // Initialize MQTT over the WebSocket client.
  mqtt.begin(client);

  String clientId =
      "PicoW-" + String((uint32_t)micros(), HEX);

  Serial.println("Connecting MQTT client...");

  if (!mqtt.connect(clientId.c_str())) {
    Serial.println("Initial MQTT connection failed.");
    Serial.println("Check broker connectivity and library compatibility.");
  } else {
    Serial.println("MQTT connected!");

    mqtt.subscribe(
      subscribeTopic,
      [](const String& payload, const size_t size) {
        Serial.print("Received command: ");
        Serial.println(payload);
      }
    );

    Serial.print("Subscribed to: ");
    Serial.println(subscribeTopic);
  }

  Serial.print("Publishing to: ");
  Serial.println(publishTopic);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(100);
    return;
  }

  // Process MQTT packets and subscription callbacks.
  mqtt.update();

  if (millis() - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = millis();

    String payload = "{\"device\":\"Pico W\",";
    payload += "\"status\":\"online\",";
    payload += "\"uptime_ms\":" + String(millis()) + "}";

    if (mqtt.connected()) {
      if (mqtt.publish(publishTopic, payload)) {
        Serial.print("Published: ");
        Serial.println(payload);
      } else {
        Serial.println("Publish failed.");
      }
    }
  }

  delay(10);
}
