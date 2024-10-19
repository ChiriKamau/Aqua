#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "karis";
const char* password = "password";

// Raspberry Pi server address (replace with your Pi's IP address)
const char* serverUrl = "http://192.168.1.100:5000/data";

// Pin definitions
const int FLOW_SENSOR_PIN = 35;  // Analog input pin for flow sensor

// Variables for water measurement
float totalVolume = 0.0;    
float flowRate = 0.0;       
float monthlyCost = 0.0;    
int valveOpenPercent = 100; 

// Variables for connection status
bool isWifiConnected = false;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000; // Send every 5 seconds

void drawValveBar(int percent) {
    int barWidth = 140;
    int barHeight = 15;
    int startX = 170;
    int startY = 210;
    
    // Draw empty bar outline
    M5.Lcd.drawRect(startX, startY, barWidth, barHeight, WHITE);
    
    // Calculate filled width based on percentage
    int fillWidth = (percent * (barWidth - 2)) / 100;
    
    // Clear previous fill
    M5.Lcd.fillRect(startX + 1, startY + 1, barWidth - 2, barHeight - 2, BLACK);
    
    // Draw new fill
    M5.Lcd.fillRect(startX + 1, startY + 1, fillWidth, barHeight - 2, ORANGE);
}

void updateDisplay() {
    // Clear display areas
    M5.Lcd.fillRect(0,30,164,104,BLACK);
    M5.Lcd.fillRect(166,30,164,104,BLACK);
    M5.Lcd.fillRect(0,136,164,104,BLACK);
    M5.Lcd.fillRect(166,136,164,104,BLACK);
    
    // First quadrant - Volume
    M5.Lcd.setCursor(5, 40);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("Volume");
    M5.Lcd.setCursor(20,80);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("%.1f", totalVolume);
    M5.Lcd.setCursor(120,80);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("L");
    
    // Second quadrant - Flow Rate
    M5.Lcd.setCursor(170,40);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("FLOW");
    M5.Lcd.setCursor(180,80);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("%.1f", flowRate);
    M5.Lcd.setCursor(270,80);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("L/m");
    
    // Third quadrant - Cost
    M5.Lcd.setCursor(5,137);
    M5.Lcd.setTextColor(BLUE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("Cost");
    M5.Lcd.setCursor(20,180);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(4);
    M5.Lcd.printf("%.2f", monthlyCost);
    M5.Lcd.setCursor(120,180);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("$");
    
    // Fourth quadrant - Valve
    M5.Lcd.setCursor(170,137);
    M5.Lcd.setTextColor(ORANGE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("Valve");
    M5.Lcd.setCursor(180,180);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(4);
    M5.Lcd.print(valveOpenPercent);
    M5.Lcd.setCursor(270,180);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("%");
    
    // Draw valve position bar
    drawValveBar(valveOpenPercent);
}

void setupWiFi() {
    WiFi.begin(ssid, password);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Connecting to WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        M5.Lcd.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isWifiConnected = true;
        M5.Lcd.println("\nConnected!");
        M5.Lcd.println(WiFi.localIP().toString());
    } else {
        M5.Lcd.println("\nFailed to connect!");
    }
    delay(2000);
}

void sendDataToPi() {
    if (!isWifiConnected || WiFi.status() != WL_CONNECTED) {
        return;
    }

    // Create JSON document
    StaticJsonDocument<200> doc;
    doc["total_volume"] = totalVolume;
    doc["flow_rate"] = flowRate;
    doc["monthly_cost"] = monthlyCost;
    doc["valve_percent"] = valveOpenPercent;

    // Serialize JSON to string
    String jsonString;
    serializeJson(doc, jsonString);

    // Send HTTP POST request
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(jsonString);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        // Small indicator for successful send
        M5.Lcd.fillCircle(300, 15, 5, GREEN);
    } else {
        // Indicator for failed send
        M5.Lcd.fillCircle(300, 15, 5, RED);
    }
    
    http.end();
}

void setup() {
    M5.begin();
    M5.Power.begin();
    
    // Initialize analog pin
    pinMode(FLOW_SENSOR_PIN, INPUT);
    
    // Initialize display
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(90, 0);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("WATER METER");
    
    // Draw layout lines
    M5.Lcd.drawLine(0, 135, 360, 135, WHITE);
    M5.Lcd.drawLine(165, 30, 165, 240, WHITE);
    M5.Lcd.drawLine(0, 30, 360, 30, WHITE);
    
    // Connect to WiFi
    setupWiFi();
}

void loop() {
    // Read flow rate directly from analog pin 35
    flowRate = analogRead(FLOW_SENSOR_PIN);
    
    // Map the flow rate to valve opening percentage (0-4095 maps to 0-100%)
    valveOpenPercent = map(flowRate, 0, 4095, 0, 100);
    valveOpenPercent = constrain(valveOpenPercent, 0, 100);
    
    // Update total volume (assuming flowRate represents L/min)
    totalVolume += (flowRate / 60.0);
    
    // Update monthly cost
    monthlyCost = totalVolume * 0.5;
    
    // Update display
    updateDisplay();
    
    // Send data to Raspberry Pi every 5 seconds
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendInterval) {
        sendDataToPi();
        lastSendTime = currentTime;
    }
    
    delay(1000);
}
