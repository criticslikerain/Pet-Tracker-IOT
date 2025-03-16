#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseESP32.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>  

// ** Firebase Configuration **
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// ** Firebase Credentials **
#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""

// ** SIM800L & GPS Configuration **
HardwareSerial sim800(2);   // TX = 16, RX = 17
HardwareSerial gpsSerial(1); // TX = 27, RX = 26
TinyGPSPlus gps;

// ** SIM Card APN Settings **
#define APN "internet.globe.com.ph"
#define USER ""
#define PASS ""

// ** WiFi Configuration (For fallback) **
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// ** Telegram Bot Settings **
#define TELEGRAM_BOT_TOKEN ""
#define TELEGRAM_CHAT_ID ""

bool gprsConnected = false;
unsigned long lastMessageID = 0;  // Track last processed message

void setup() {
    delay(5000);
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, 26, 27);
    sim800.begin(9600, SERIAL_8N1, 16, 17);

    Serial.println("Initializing SIM800L...");
    sim800.println("AT");  // Check module
    delay(1000);

    // ** Initialize Firebase Config & Auth **
    firebaseConfig.host = FIREBASE_HOST;
    firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
    
    Firebase.begin(&firebaseConfig, &firebaseAuth);
    Firebase.reconnectWiFi(true);

    gprsConnected = connectGPRS();  // Try to connect GPRS

    if (!gprsConnected) {
        connectWiFi();  // Fallback to WiFi if GPRS fails
    }
}

// ** Function to connect to GPRS **
bool connectGPRS() {
    Serial.println("Checking SIM Card...");

    sim800.println("AT+CPIN?"); // Check if SIM card is ready
    delay(1000);
    if (sim800.find("READY")) {
        Serial.println("✅ SIM Card Detected!");
    } else {
        Serial.println("❌ No SIM Card or Not Ready!");
        return false;
    }

    sim800.println("AT+CREG?"); // Check if registered to network
    delay(1000);
    if (sim800.find("+CREG: 0,1") || sim800.find("+CREG: 0,5")) {
        Serial.println("✅ Registered to Network!");
    } else {
        Serial.println("❌ Not Registered! Check SIM signal.");
        return false;
    }

    Serial.println("Connecting to GPRS...");
    sim800.println("AT+CGATT=1");  // Enable GPRS
    delay(1000);
    
    sim800.println("AT+CSTT=\"" APN "\",\"" USER "\",\"" PASS "\"");  // Set APN
    delay(5000);
    
    sim800.println("AT+CIICR");  // Activate connection
    delay(5000);
    
    sim800.println("AT+CIFSR");  // Get IP address
    delay(2000);
    
    sim800.println("AT+CGATT?");
    delay(1000);
    if (sim800.find("1")) {
        Serial.println("✅ GPRS Connected!");
        return true;
    } else {
        Serial.println("❌ GPRS Connection Failed!");
        return false;
    }
}

// ** Function to connect to WiFi **
void connectWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        Serial.print(".");
        delay(1000);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi Connected!");
    } else {
        Serial.println("\n❌ WiFi Connection Failed!");
    }
}

// ** Function to check Telegram Messages **
void checkTelegramMessages() {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();  // ✅ Bypass SSL for ESP32

    HTTPClient http;
    String url = "https://api.telegram.org/bot" TELEGRAM_BOT_TOKEN "/getUpdates?offset=-1";

    http.begin(client, url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.println("📩 Received Telegram Data:");
        Serial.println(payload);
        
        processTelegramMessage(payload);
    } else {
        Serial.print("❌ Failed to get messages. Error: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
}

// ** Function to process Telegram Message **
void processTelegramMessage(String payload) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    if (!doc.containsKey("result")) return;

    JsonArray results = doc["result"].as<JsonArray>();
    if (results.size() == 0) return;

    JsonObject lastMessage = results[results.size() - 1]["message"];
    unsigned long messageID = lastMessage["message_id"];
    String text = lastMessage["text"];

    if (messageID == lastMessageID) return; // Ignore duplicate messages
    lastMessageID = messageID;

    Serial.print("📨 New Message: ");
    Serial.println(text);

    if (text.equalsIgnoreCase("Locate")) {
        sendPetLocation();
    }
}

// ** Function to get the current GPS location and send it to Telegram **
void sendPetLocation() {
    float latitude = 0.0, longitude = 0.0;
    
    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        gps.encode(c);
        if (gps.location.isUpdated()) {
            latitude = gps.location.lat();
            longitude = gps.location.lng();
            break;  // Exit loop once we get a valid location
        }
    }

    if (latitude == 0.0 || longitude == 0.0) {
        Serial.println("❌ No valid GPS data!");
        return;
    }

    sendToTelegram(latitude, longitude);
}

// ** Function to send location to Telegram **
void sendToTelegram(float latitude, float longitude) {
    if (WiFi.status() != WL_CONNECTED && !gprsConnected) {
        Serial.println("❌ No internet available for Telegram!");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = "https://api.telegram.org/bot" TELEGRAM_BOT_TOKEN "/sendMessage?chat_id=" TELEGRAM_CHAT_ID "&text=📍 Pet Location: http://maps.google.com/maps?q=" + String(latitude, 6) + "," + String(longitude, 6);
    
    Serial.println("Sending location to Telegram...");
    http.begin(client, url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        Serial.println("✅ Telegram Message Sent!");
    } else {
        Serial.print("❌ Telegram Message Failed! HTTP Code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

// ** Main loop: Only checks for Telegram messages every 5 seconds **
void loop() {
    checkTelegramMessages();
    delay(1000); // Check every 5 seconds
}
