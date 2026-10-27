
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

  Serial1.print("Connecting to Wi-Fi");

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 30000) {
    delay(500);
    Serial1.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial1.println("\nWi-Fi connection failed.");
    return false;
  }

  Serial1.println("\nWi-Fi connected!");
  Serial1.print("IP address: ");
  Serial1.println(WiFi.localIP());

  return true;
}

// MQTT message callback
void onCommand(const String& payload, const size_t size) {
  Serial1.print("Command received: ");
  Serial1.println(payload);
}

// Connect to MQTT
bool connectMQTT() {
  String clientId =
      "PicoW-" + String((uint32_t)micros(), HEX);

  Serial1.println("Connecting to MQTT...");

  if (!mqttClient.connect(clientId)) {
    Serial1.println("MQTT connection attempt failed.");
    return false;
  }

  Serial1.println("MQTT connected!");

  if (mqttClient.subscribe(subscribeTopic, onCommand)) {
    Serial1.print("Subscribed to: ");
    Serial1.println(subscribeTopic);
  } else {
    Serial1.println("Subscription failed.");
  }

  return true;
}

void setup() {
  Serial1.begin(115200);
  delay(2000);

  Serial1.println("\nPico W HiveMQ MQTT over WebSockets");

  publishTopic = String(TOPIC_PREFIX) + "/data";
  subscribeTopic = String(TOPIC_PREFIX) + "/command";

  if (!connectWiFi()) {
    return;
  }

  // Start WebSocket transport. "mqtt" is the WebSocket subprotocol.
  wsClient.begin(MQTT_HOST, MQTT_PORT, MQTT_PATH, "mqtt");
  wsClient.setReconnectInterval(5000);

  mqttClient.begin(wsClient);

  Serial1.print("Publish topic: ");
  Serial1.println(publishTopic);

  Serial1.print("Subscribe topic: ");
  Serial1.println(subscribeTopic);

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
      Serial1.print("Published: ");
      Serial1.println(payload);
    } else {
      Serial1.println("Publish failed.");
    }
  }

  delay(10);
}
