#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"  //DHT Sensor Library

//Signal Pin
#define sIgnalLed 16
// Define the pins used by the LoRa module
#define ss 5
#define rst 14
#define dio0 2

//defining DHT11 
#define DHTPIN 21   
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//defining Soil Moisture Sensor
int _moisture,sensor_analog;
const int sensor_pin = 34;  /* Soil moisture sensor O/P pin */

//defining Raindrop Senor
//Digital Pin
#define DO_PIN 4
//Analog Pin
#define AO_PIN 35

//defining MQ-2 Air Sensor
//Digita Pin
#define DO_PIN2 27
//Analog Pin
#define AO_PIN2 39

//defining Sound Sensor
//Digital PIn
#define digi_SENSOR 13
int currentState;  
//Analog Pin
#define analog_SENSOR_PIN 36

//LoRa Variables
String LoRaMessage = "";
int counter = 0;

//sensor values initialization
float Soil_Moisture = 0.0;
float Env_Temperature = 0.0;
float Env_Humidity = 0.0;
int Rain_Present=0;
float Rain_Intensity=0.0;
int Flammable_Gas=0;
float Flammable_Gas_Intensity=0.0;
int Sound_Present=0;
float Sound_Intensity=0.0;

// Sensor setup (dummy sensor data for illustration)

//Soil_Moisture_data from SoilMoisture Sensor
float Soil_Moisture_data()
{
  sensor_analog = analogRead(sensor_pin);
  _moisture = ( 100 - ( (sensor_analog/4095.00) * 100 ) );
  return _moisture; // "_moisture" in %
}

float Env_Temperature_data()
{
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {
    Serial.println(F("Failed to read TEMPERATURE from DHT sensor!"));
    return 0;
  }
  return t;  // "t" in degree celcius
}
float Env_Humidity_data()
{
  float h = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h)) {
    Serial.println(F("Failed to read HUMIDITY from DHT sensor!"));
    return 0;
  }
  return h; // "h" in %
}
int Rain_Present_data()
{
  int rain_state = digitalRead(DO_PIN);
  if (rain_state == HIGH)
    return 0;
  else
    return 1;
}
float Rain_Intensity_data()
{
  int rain_value = analogRead(AO_PIN);
  return 4095-rain_value;
}
int Flammable_Gas_Present_data()
{
  int gasState = digitalRead(DO_PIN2);
  if (gasState == HIGH)
    return 0;
  else
    return 1;
}
float Flammable_Gas_Intensity_data()
{
  int gasValue = analogRead(AO_PIN2);
  return gasValue;
}
int Sound_Present_data()
{
  // Read the current state of the sound sensor digital pin
  int currentState = digitalRead(digi_SENSOR);

  // Return `1` if sound is detected (LOW state), otherwise `0`
  delay(10);
  return (currentState == LOW) ? 1 : 0;
}
float Sound_Intensity_data()
{
  int soundIntensity = analogRead(analog_SENSOR_PIN);
  return soundIntensity;
}
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  // Attempt to start LoRa module
  while (!LoRa.begin(433E6)) { // Use 433E6, 866E6, or 915E6 based on your region
    Serial.println("LoRa initialization failed. Trying again...");
    delay(500);
  }

  // Change sync word (0xF3) to match the receiver
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(7);

  Serial.println("LoRa Sendr Initialized Successfully!");

  //start the dht11 sensor
  dht.begin();

  //Setting Raindrop Sensor
  //Digital Pin
  pinMode(DO_PIN, INPUT);
  //Analog Pin
  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetPinAttenuation(AO_PIN, ADC_11db);

  //Setting MQ-2 Air Sensor
  //Digital Pin
  pinMode(DO_PIN2, INPUT);
  //Analog Pin
  // set the ADC attenuation to 11 dB (up to ~3.3V input)
  analogSetPinAttenuation(AO_PIN, ADC_11db);
  Serial.println("Warming up the MQ2 sensor");
  delay(1000);  // wait for the MQ2 to warm up

  //Setting Sound Sensor
  //Digital Pin
  pinMode(digi_SENSOR, INPUT);
  //Analog Pin
  pinMode(analog_SENSOR_PIN, INPUT);

  //Setting Signal Led
  pinMode(sIgnalLed, OUTPUT);
  digitalWrite(sIgnalLed, LOW);
}

void loop() {
  Soil_Moisture=Soil_Moisture_data();
  Env_Temperature=Env_Temperature_data();
  Env_Humidity=Env_Humidity_data();
  Rain_Present=Rain_Present_data();
  Rain_Intensity=Rain_Intensity_data();
  Flammable_Gas=Flammable_Gas_Present_data();
  Flammable_Gas_Intensity=Flammable_Gas_Intensity_data();
  Sound_Present=Sound_Present_data();
  Sound_Intensity=Sound_Intensity_data();
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


LoRaMessage = String(counter) + "*" + String(Soil_Moisture) +"!" + String(Env_Temperature) + "@" + String(Env_Humidity)+ "#" + String(Rain_Present) + "$" + String(Rain_Intensity) 
              + "%" + String(Flammable_Gas) + "^" +  String(Flammable_Gas_Intensity) + "&" + String(Sound_Present) + "-" +  String(Sound_Intensity);

  // Send data over LoRa
  Serial.print("Sending packet: ");
  Serial.println(LoRaMessage);
  digitalWrite(sIgnalLed, HIGH);
  LoRa.beginPacket();
  LoRa.print(LoRaMessage);
  LoRa.endPacket();
  digitalWrite(sIgnalLed, LOW);
  // Debounce the button
  delay(20000);
  counter++;
  
}