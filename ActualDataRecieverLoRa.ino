#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "time.h"
#include <SPI.h>
#include <LoRa.h>

// Define the pins used by the LoRa module
#define ss 15
#define rst 16
#define dio0 2
#define ledg D3

//Sensor Data Initialization
int counter = 0;
float Soil_Moisture = 0.0;
float Env_Temperature = 0.0;
float Env_Humidity = 0.0;
int Rain_Present=0;
float Rain_Intensity=0.0;
int Flammable_Gas=0;
float Flammable_Gas_Intensity=0.0;
int Sound_Present=0;
float Sound_Intensity=0.0;

// WiFi credentials
const char* WIFI_SSID = "real";
const char* WIFI_PASSWORD = "12345678";

//For fetching time from ntp server
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;           // Adjust this to your time zone's GMT offset
const int   daylightOffset_sec = 0;      // Adjust for daylight saving if applicable

// Firebase credentials
const String DATABASE_URL = "https://temppicow-default-rtdb.asia-southeast1.firebasedatabase.app/";  // Replace with your Firebase Realtime Database URL
const String FIREBASE_AUTH = "0avkBIfd68SUtRzaf7uilKqRAu2JpqeYhAZIM4Jn";  // Optional for basic security

WiFiClientSecure wifiClient; // Use WiFiClientSecure for HTTPS

// Connect to WiFi
void connect_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Send data to Firebase Realtime Database
void send_data(const String& path, const String& jsonData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = DATABASE_URL + path + ".json?auth=" + FIREBASE_AUTH;

    // Start HTTP connection with WiFiClientSecure
    wifiClient.setInsecure(); // Use insecure connection; disable SSL verification for testing
    http.begin(wifiClient, url); // Correct HTTPS connection
    
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.PUT(jsonData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Data sent to Firebase");
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error sending data. HTTP response code: " + String(httpResponseCode));
    }

    http.end();  // End HTTP connection
  } else {
    Serial.println("WiFi Disconnected");
  }
}

//Fetching time thorugh NTP servers
String printLocalTime()
{
   struct tm timeinfo;
   if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return "";
   }

   // Create a buffer to hold the formatted time
   char timeStringBuff[64];
   
   // Format the time into the buffer
   strftime(timeStringBuff, sizeof(timeStringBuff), "%d-%b-%Y//%H:%M:%S", &timeinfo);

   // Print the formatted time
   Serial.println(timeStringBuff);
   return String(timeStringBuff);
}
void setup() {
  pinMode(ledg, OUTPUT);
  digitalWrite(ledg, LOW);
  // Initialize Serial Monitor
  Serial.begin(115200);

  connect_wifi();

  // Initialize and get the time from NTP
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
   printLocalTime();

  while (!Serial);

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  // Attempt to start LoRa module
  while (!LoRa.begin(433E6)) { // Use 433E6, 866E6, or 915E6 based on your region
    Serial.println("LoRa initialization failed. Trying again...");
    delay(500);
  }

  // Change sync word (0xF3) to match the sender
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(7);

  Serial.println("LoRa Initialized Successfully!");
  Serial.println("Waiting for messages...");
}

void loop() {
  int pos1, pos2, pos3, pos4, pos5, pos6, pos7, pos8, pos9;
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet:  ");
    String LoRaData = LoRa.readString();
    Serial.print(LoRaData);
    // read packet
    while (LoRa.available()) {
      Serial.println((char)LoRa.read());
    }

    pos1 = LoRaData.indexOf('*');
    pos2 = LoRaData.indexOf('!');
    pos3 = LoRaData.indexOf('@');
    pos4 = LoRaData.indexOf('#');
    pos5 = LoRaData.indexOf('$');
    pos6 = LoRaData.indexOf('%');
    pos7 = LoRaData.indexOf('^');
    pos8 = LoRaData.indexOf('&');
    pos9 = LoRaData.indexOf('-');

    counter = LoRaData.substring(0, pos1).toInt();
    Soil_Moisture = LoRaData.substring(pos1 + 1, pos2).toInt();
    Env_Temperature = LoRaData.substring(pos2 + 1, pos3).toInt();
    Env_Humidity = LoRaData.substring(pos3 + 1, pos4).toInt();
    Rain_Present = LoRaData.substring(pos4 + 1, pos5).toInt();
    Rain_Intensity = LoRaData.substring(pos5 + 1, pos6).toInt();
    Flammable_Gas = LoRaData.substring(pos6 + 1, pos7).toInt();
    Flammable_Gas_Intensity = LoRaData.substring(pos7 + 1, pos8).toInt();
    Sound_Present = LoRaData.substring(pos8 + 1, pos9).toInt();
    Sound_Intensity = LoRaData.substring(pos9 + 1, LoRaData.length()).toInt();

    //0
    Serial.print("Packet No. :");
    Serial.println(counter);
    //1
  Serial.print("Soil_Moisture: ");
  Serial.println(Soil_Moisture);
  //2
  Serial.print("Env_Temperature: ");
  Serial.println(Env_Temperature);
  //3
  Serial.print("Env_Humidity: ");
  Serial.println(Env_Humidity);
  //4
  Serial.print("Rain_Present: ");
  Serial.println(Rain_Present);
  //5
  Serial.print("Rain_Intensity: ");
  Serial.println(Rain_Intensity);
  //6
  Serial.print("Flammable_Gas: ");
  Serial.println(Flammable_Gas);
  //7
  Serial.print("Flammable_Gas_Intensity: ");
  Serial.println(Flammable_Gas_Intensity);
  //8
  Serial.print("Sound_Present: ");
  Serial.println(Sound_Present);
  //9
  Serial.print("Sound_Intensity: ");
  Serial.println(Sound_Intensity);

    // Get timestamp
    String formattedTime = printLocalTime();

    // Create JSON data
    DynamicJsonDocument doc(256);
    doc["Soil_Moisture"] = Soil_Moisture;
    doc["Env_Temperature"] = Env_Temperature;
    doc["Env_Humidity"] = Env_Humidity;
    doc["Rain_Present"] = Rain_Present;
    doc["Rain_Intensity"] = Rain_Intensity;
    doc["Flammable_Gas"] = Flammable_Gas;
    doc["Flammable_Gas_Intensity"] = Flammable_Gas_Intensity;
    doc["Sound_Present"] = Sound_Present;
    doc["Sound_Intensity"] = Sound_Intensity;
    String jsonData;
    serializeJson(doc, jsonData);

    // Send data to Firebase
    send_data("SensorData/Node1/" + formattedTime, jsonData);
    
    if (Soil_Moisture==1){
      delay(100);
      digitalWrite(ledg,HIGH);
    }
    delay(500);
    digitalWrite(ledg,LOW);
    
    // Print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    Serial.println();
  }
}