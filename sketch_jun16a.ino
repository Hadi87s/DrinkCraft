#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "zahi";
const char* password = "osama2002";

// Start the server on port 80
ESP8266WebServer server(80);

// CORS headers for all responses
void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
}

// Handle CORS preflight
void handleOptions() {
  addCORSHeaders();
  server.send(204); // No Content
}

// Handle the POST /order endpoint
void handleOrder() {
  addCORSHeaders(); // MUST come before send()

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error || !doc.containsKey("order")) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  int order = doc["order"];
  Serial.print("Order received: ");
  Serial.println(order);

  server.send(200, "application/json", "{\"status\":\"received\"}");
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  // Route registration
  server.on("/order", HTTP_OPTIONS, handleOptions); // Preflight
  server.on("/order", HTTP_POST, handleOrder);      // Actual order

  server.begin();
  Serial.println("Server started on port 80");
}

void loop() {
  server.handleClient();
}
