
#include <WiFi.h>
#include <WebSocketsClient.h>  // Include before MQTTPubSubClient.h
#include <MQTTPubSubClient.h>

// Wi-Fi credentials
const char* WIFI_SSID = "Logicon-GUEST";
const char* WIFI_PASSWORD = "";

// HiveMQ public WebSocket broker
const char* MQTT_HOST = "broker.hivemq.com";
const uint16_t MQTT_PORT = 8000;
const char* MQTT_PATH = "/mqtt";

// Use a unique topic prefix
const char* TOPIC_PREFIX = "pico-w-demo-change-this-74291";

String publishTopic;
String subscribeTopic;

WebSocketsClient wsClient;

// IMPORTANT: Do not name this object "mqtt",
// because the library also defines a namespace named mqtt.
MQTTPubSubClient mqttClient;

unsigned long lastPublish = 0;
unsigned long lastConnectAttempt = 0;

const unsigned long PUBLISH_INTERVAL = 5000;
const unsigned long RECONNECT_INTERVAL = 3000;

// Wi-Fi connection
bool connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");

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

// MQTT message callback
void onCommand(const String& payload, const size_t size) {
  Serial.print("Command received: ");
  Serial.println(payload);
}

// Connect to MQTT
bool connectMQTT() {
  String clientId =
      "PicoW-" + String((uint32_t)micros(), HEX);

  Serial.println("Connecting to MQTT...");

  if (!mqttClient.connect(clientId)) {
    Serial.println("MQTT connection attempt failed.");
    return false;
  }

  Serial.println("MQTT connected!");

  if (mqttClient.subscribe(subscribeTopic, onCommand)) {
    Serial.print("Subscribed to: ");
    Serial.println(subscribeTopic);
  } else {
    Serial.println("Subscription failed.");
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\nPico W HiveMQ MQTT over WebSockets");

  publishTopic = String(TOPIC_PREFIX) + "/data";
  subscribeTopic = String(TOPIC_PREFIX) + "/command";

  if (!connectWiFi()) {
    return;
  }

  // Start WebSocket transport. "mqtt" is the WebSocket subprotocol.
  wsClient.begin(MQTT_HOST, MQTT_PORT, MQTT_PATH, "mqtt");
  wsClient.setReconnectInterval(5000);

  mqttClient.begin(wsClient);

  Serial.print("Publish topic: ");
  Serial.println(publishTopic);

  Serial.print("Subscribe topic: ");
  Serial.println(subscribeTopic);

  // MQTT connection is retried in loop().
  lastConnectAttempt = millis() - RECONNECT_INTERVAL;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(100);
    return;
  }

  // Process incoming MQTT data and callbacks.
  mqttClient.update();

  if (!mqttClient.isConnected()) {
    if (millis() - lastConnectAttempt >= RECONNECT_INTERVAL) {
      lastConnectAttempt = millis();
      connectMQTT();
    }

    delay(10);
    return;
  }

  if (millis() - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = millis();

    String payload = "{\"device\":\"Pico W\",";
    payload += "\"status\":\"online\",";
    payload += "\"uptime_ms\":";
    payload += String(millis());
    payload += "}";

    if (mqttClient.publish(publishTopic, payload)) {
      Serial.print("Published: ");
      Serial.println(payload);
    } else {
      Serial.println("Publish failed.");
    }
  }

  delay(10);
}
