
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
  Serial1.begin(115200);
  delay(2000);

  Serial1.println("\n--- Pico W JSON CRUD Demo ---");

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial1.print("Connecting to Wi-Fi");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 30000) {
    delay(500);
    Serial1.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial1.println("\nWi-Fi connection failed.");
    Serial1.println("Check SSID, password and network security.");
    return;
  }

  Serial1.println("\nWi-Fi connected!");
  Serial1.print("IP address: ");
  Serial1.println(WiFi.localIP());

  createTodo();
  readTodo(1);
  updateTodo(1);
  deleteTodo(1);

  Serial1.println("\n--- CRUD Demo Finished ---");
}

void loop() {
  // Operations run once in setup().
}

// Common response/error reporting
void printResponse(HTTPClient& http, int code) {
  Serial1.print("HTTP status: ");
  Serial1.println(code);

  if (code > 0) {
    String response = http.getString();
    Serial1.println("Response:");
    Serial1.println(response);
  } else {
    Serial1.print("HTTP error: ");
    Serial1.println(http.errorToString(code));
  }
}

// 1. CREATE: POST /todos
void createTodo() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.setInsecure();  // Test only: skips certificate verification

  Serial1.println("\n[POST] Creating todo...");

  if (!http.begin(String(base_url))) {
    Serial1.println("Could not start HTTP connection.");
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

  Serial1.print("\n[GET] Reading todo ");
  Serial1.println(id);

  if (!http.begin(url)) {
    Serial1.println("Could not start HTTP connection.");
    return;
  }

  int code = http.GET();

  if (code > 0) {
    Serial1.print("HTTP status: ");
    Serial1.println(code);

    if (code == HTTP_CODE_OK) {
      JsonDocument doc;
      DeserializationError error =
          deserializeJson(doc, http.getStream());

      if (!error) {
        Serial1.print("User ID: ");
        Serial1.println(doc["userId"] | 0);

        Serial1.print("Todo ID: ");
        Serial1.println(doc["id"] | 0);

        Serial1.print("Title: ");
        Serial1.println(doc["title"] | "");

        Serial1.print("Completed: ");
        Serial1.println((doc["completed"] | false)
                           ? "Yes" : "No");
      } else {
        Serial1.print("JSON parsing failed: ");
        Serial1.println(error.c_str());
      }
    } else {
      Serial1.println(http.getString());
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

  Serial1.print("\n[PUT] Updating todo ");
  Serial1.println(id);

  if (!http.begin(url)) {
    Serial1.println("Could not start HTTP connection.");
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

  Serial1.print("\n[DELETE] Deleting todo ");
  Serial1.println(id);

  if (!http.begin(url)) {
    Serial1.println("Could not start HTTP connection.");
    return;
  }

  int code = http.sendRequest("DELETE");
  printResponse(http, code);

  http.end();
}
