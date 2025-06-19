#include <esp_wifi.h>
#include <esp_wpa2.h>  // Needed for WPA2-Enterprise
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char *ssid = "zahi";
const char *password = "osama2002";

// Use WebServer for easier API implementation
WebServer server(80);


const char *identity = "12113636";  // university username
const char *username = "12113636";  // often same as identity

/*
void connectToWPA2Enterprise() {
    WiFi.disconnect(true); // clean start
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid); // Start WiFi with SSID only

    esp_wifi_sta_wpa2_ent_enable(); // Enable WPA2-Enterprise

    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)identity, strlen(identity));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));

    esp_wifi_connect();

    Serial.print("Connecting to WPA2-Enterprise network");

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30) {
        delay(500);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WPA2-Enterprise WiFi.");
    }
}*/

void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}



void setup() {
  Serial.begin(9600);  // USB Serial Monitor
  // Use Serial2 for Arduino comms (pins 16/17)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17 for Arduino communication

  pinMode(2, OUTPUT);  // set the LED pin mode (using pin 2)

  Serial.println("ESP32 ready");
  Serial2.println("ESP32 connected to Arduino");  // Send message to Arduino

  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);  // Explicitly set station mode
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Also send WiFi status to Arduino
  Serial2.println("WiFi connected");
  Serial2.print("IP: ");
  Serial2.println(WiFi.localIP().toString());



  // Set up API endpoints
  setupAPIEndpoints();

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

// Handle API requests and bridge them to Arduino
String sendCommandToArduino(const String &command, const String &payload) {
  // Clean up the payload - remove whitespace and format it more consistently
  String cleanPayload = payload;
  cleanPayload.replace("\n", "");
  cleanPayload.replace("\r", "");
  cleanPayload.trim();

  String fullCommand = command + ":" + cleanPayload;

  while (Serial2.available()) Serial2.read();

  // Send command to Arduino and flush to ensure it's fully sent
  Serial.println("Sending to Arduino: " + fullCommand);
  Serial2.println(fullCommand);
  //delay(200); // Short delay to let Arduino process

  // Wait for response (timeout after 15 seconds)
  unsigned long startTime = millis();
  String response = "";

  // Clear any leftover data in the buffer
  // while (Serial2.available())
  // {
  //     Serial.println("this charcater is dumb : " + Serial2.read());
  // }

  // Wait for response
  while ((millis() - startTime) < 15000) {
    if (Serial2.available()) {
      char c = Serial2.read();

      if (c == '\n') {
        // End of response
        break;
      }
      // Only add printable characters to avoid control characters
      if (c >= 32 && c <= 126) {
        response += c;
      }
    }
    // notice it
    delay(10);
  }

  response.trim();

  if (response.length() == 0) {
    Serial.println("No response from Arduino (timeout)");
    return "{\"success\":false,\"reason\":\"Timeout or no response from Arduino\"}";
  }

  // Clean the response of any non-JSON characters at the beginning
  int jsonStart = response.indexOf('{');
  if (jsonStart > 0) {
    response = response.substring(jsonStart);
  }

  Serial.println("Arduino response: " + response);
  return response;
}

// Generic handler for all API endpoints
void handleAPIRequest() {
  addCORSHeaders();
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"success\":false,\"reason\":\"Method not allowed\"}");
    return;
  }

  // Extract command from URI
  String uri = server.uri();
  String command = uri.substring(5);  // Remove "/api/" prefix

  // Get payload
  String payload = server.arg("plain");

  // Send command to Arduino and get response
  String response = sendCommandToArduino(command, payload);

  // Send response back to client
  server.send(200, "application/json", response);
}

// Handle root request
void handleRoot() {
  server.send(200, "text/plain", "Tinker Blocks ESP32 API Bridge");
}

void handleOrderRequest() {
  addCORSHeaders();

  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"success\":false,\"reason\":\"Method not allowed\"}");
    return;
  }

  String payload = server.arg("plain");
  Serial.println("Received JSON from frontend: " + payload);

  // Optionally parse it (if you need to extract only the orderId)
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.println("JSON parse failed");
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int orderId = doc["order"];
  int toppingsId = doc["toppings"];  // ✅ new field
  Serial.println("Forwarding to Arduino -> Order: " + String(orderId) + ", Toppings: " + String(toppingsId));

  // Send as CSV-style string: "3,2"
  Serial2.println(String(orderId) + "," + String(toppingsId));


  server.send(200, "application/json", "{\"status\":\"Order forwarded to Arduino\"}");
}


// Setup API endpoints
void setupAPIEndpoints() {
  server.on("/order", HTTP_OPTIONS, []() {
    addCORSHeaders();
    server.send(204);  // No Content
  });

  server.on("/order", HTTP_POST, handleOrderRequest);

  // Repeat this for all /api/* endpoints
  const char *apiEndpoints[] = {
    "/api/move", "/api/rotate", "/api/pen",
    "/api/gyro", "/api/sensor", "/api/ir", "/api/buzzer"
  };

  for (const char *endpoint : apiEndpoints) {
    server.on(endpoint, HTTP_OPTIONS, []() {
      addCORSHeaders();
      server.send(204);
    });
    server.on(endpoint, HTTP_POST, handleAPIRequest);
  }
  server.on("/", HTTP_GET, handleRoot);

  // Use a single handler for all API endpoints
  server.on("/api/move", HTTP_POST, handleAPIRequest);
  server.on("/api/rotate", HTTP_POST, handleAPIRequest);
  server.on("/api/pen", HTTP_POST, handleAPIRequest);
  server.on("/api/gyro", HTTP_POST, handleAPIRequest);
  server.on("/api/sensor", HTTP_POST, handleAPIRequest);
  server.on("/api/ir", HTTP_POST, handleAPIRequest);
  server.on("/api/buzzer", HTTP_POST, handleAPIRequest);
  server.on("/order", HTTP_POST, handleOrderRequest);

  // Handle not found
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"success\":false,\"reason\":\"Endpoint not found\"}");
  });
}
