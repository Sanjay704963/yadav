#include <WiFiS3.h>
#include <ArduinoHttpClient.h>

// Wi-Fi credentials
const char* ssid = "Three_7AD76E";
const char* password = "2vLuswu235z3256";

// Raspberry Pi server IP and port
const char* serverAddress = "192.168.0.196"; // Replace with Raspberry Pi IP
const int serverPort = 5000;

// Define MQ-9B and buzzer pins
const int MQ9B_PIN = A0;
const int BUZZER_PIN = 8;
const int THRESHOLD = 400;

// Create WiFi and HTTP client objects
WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, serverAddress, serverPort);

void setup() {
  // Start serial communication
  Serial.begin(9600);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // Initialize pins
  pinMode(MQ9B_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  // Read MQ-9B sensor value
  int sensorValue = analogRead(MQ9B_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);
  bool alert = sensorValue > THRESHOLD;

  // Control the buzzer
  if (alert) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Send data to the Raspberry Pi
  if (WiFi.status() == WL_CONNECTED) {
    // Prepare JSON payload
    String jsonData = String("{\"mq9_value\":") + sensorValue +
                      ",\"mq9_voltage\":" + voltage +
                      ",\"alert\":\"" + (alert ? "HIGH" : "LOW") + "\"}";

    // Send HTTP POST request
    Serial.println("Sending data to server...");
    httpClient.beginRequest();
    httpClient.post("/update");
    httpClient.sendHeader("Content-Type", "application/json");
    httpClient.sendHeader("Content-Length", jsonData.length());
    httpClient.beginBody();
    httpClient.print(jsonData);
    httpClient.endRequest();

    // Get the server response
    int statusCode = httpClient.responseStatusCode();
    String response = httpClient.responseBody();
    Serial.print("Status Code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
  } else {
    Serial.println("Wi-Fi disconnected!");
  }

  delay(1000);
}
