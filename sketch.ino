
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// -------- Wi-Fi Configuration --------
const char* ssid = "Logicon-GUEST";
const char* password = "";

// Correct base URL: no todo ID here
const char* base_url = "https://jsonplaceholder.typicode.com/todos";

// -------- Function Prototypes --------
void createTodo();
void readTodo(int id);
void updateTodo(int id);
void deleteTodo(int id);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n--- Pico W JSON CRUD Demo ---");

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWi-Fi connection failed.");
    Serial.println("Check SSID, password and network security.");
    return;
  }

  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  createTodo();
  readTodo(1);
  updateTodo(1);
  deleteTodo(1);

  Serial.println("\n--- CRUD Demo Finished ---");
}

void loop() {
  // Operations run once in setup().
}

// Common response/error reporting
void printResponse(HTTPClient& http, int code) {
  Serial.print("HTTP status: ");
  Serial.println(code);

  if (code > 0) {
    String response = http.getString();
    Serial.println("Response:");
    Serial.println(response);
  } else {
    Serial.print("HTTP error: ");
    Serial.println(http.errorToString(code));
  }
}

// 1. CREATE: POST /todos
void createTodo() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.setInsecure();  // Test only: skips certificate verification

  Serial.println("\n[POST] Creating todo...");

  if (!http.begin(String(base_url))) {
    Serial.println("Could not start HTTP connection.");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  JsonDocument doc;
  doc["userId"] = 1;
  doc["title"] = "Buy milk from Pico W";
  doc["completed"] = false;

  String body;
  serializeJson(doc, body);

  int code = http.POST(body);
  printResponse(http, code);

  http.end();
}

// 2. READ: GET /todos/1
void readTodo(int id) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.setInsecure();

  String url = String(base_url) + "/" + String(id);

  Serial.print("\n[GET] Reading todo ");
  Serial.println(id);

  if (!http.begin(url)) {
    Serial.println("Could not start HTTP connection.");
    return;
  }

  int code = http.GET();

  if (code > 0) {
    Serial.print("HTTP status: ");
    Serial.println(code);

    if (code == HTTP_CODE_OK) {
      JsonDocument doc;
      DeserializationError error =
          deserializeJson(doc, http.getStream());

      if (!error) {
        Serial.print("User ID: ");
        Serial.println(doc["userId"] | 0);

        Serial.print("Todo ID: ");
        Serial.println(doc["id"] | 0);

        Serial.print("Title: ");
        Serial.println(doc["title"] | "");

        Serial.print("Completed: ");
        Serial.println((doc["completed"] | false)
                           ? "Yes" : "No");
      } else {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.println(http.getString());
    }
  } else {
    printResponse(http, code);
  }

  http.end();
}

// 3. UPDATE: PUT /todos/1
void updateTodo(int id) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.setInsecure();

  String url = String(base_url) + "/" + String(id);

  Serial.print("\n[PUT] Updating todo ");
  Serial.println(id);

  if (!http.begin(url)) {
    Serial.println("Could not start HTTP connection.");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  JsonDocument doc;
  doc["userId"] = 1;
  doc["id"] = id;
  doc["title"] = "Task updated by Pico W";
  doc["completed"] = true;

  String body;
  serializeJson(doc, body);

  int code = http.PUT(body);
  printResponse(http, code);

  http.end();
}

// 4. DELETE: DELETE /todos/1
void deleteTodo(int id) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.setInsecure();

  String url = String(base_url) + "/" + String(id);

  Serial.print("\n[DELETE] Deleting todo ");
  Serial.println(id);

  if (!http.begin(url)) {
    Serial.println("Could not start HTTP connection.");
    return;
  }

  int code = http.sendRequest("DELETE");
  printResponse(http, code);

  http.end();
}
