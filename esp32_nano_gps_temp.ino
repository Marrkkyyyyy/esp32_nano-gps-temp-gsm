#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Adafruit_AHTX0.h>

#define PHONE_NUMBER "+639691877036"

#define RX1PIN D2  
#define TX1PIN D3 

TinyGPSPlus gps; 
Adafruit_AHTX0 aht;
SoftwareSerial sim(D0, D1);

sensors_event_t humidity, temp;

void setup() {
  delay(7000);
  Serial.begin(9600);
  sim.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RX1PIN, TX1PIN);
  if (!aht.begin()) {
    Serial.println("Could not find AHT10? Check wiring");
  }
  Serial.println("System Started...");
  Serial.println("AHT10 found");
  RecieveMessage();
  sendSMS(PHONE_NUMBER, "GSM is ready!");
}
static unsigned long lastLocationCheck = 0;
const unsigned long locationCheckInterval = 5000;
void loop() {
  
  aht.getEvent(&humidity, &temp);

  
  if (Serial1.available() > 0) {
    if (gps.encode(Serial1.read())) {
      if (gps.location.isValid()) {
        Serial.print(F("- latitude: "));
        Serial.println(gps.location.lat());
        Serial.print(F("- longitude: "));
        Serial.println(gps.location.lng());
        Serial.print(F("- altitude: "));
        if (gps.altitude.isValid())
          Serial.println(gps.altitude.meters());
        else
          Serial.println(F("INVALID"));
      } else {
        Serial.println(F("- location: INVALID"));
      }
      Serial.print(F("- speed: "));
      if (gps.speed.isValid()) {
        Serial.print(gps.speed.kmph());
        Serial.println(F(" km/h"));
      } else {
        Serial.println(F("INVALID"));
      }
      Serial.print(F("- GPS date&time: "));
      if (gps.date.isValid() && gps.time.isValid()) {
        Serial.print(gps.date.year());
        Serial.print(F("-"));
        Serial.print(gps.date.month());
        Serial.print(F("-"));
        Serial.print(gps.date.day());
        Serial.print(F(" "));
        Serial.print(gps.time.hour());
        Serial.print(F(":"));
        Serial.print(gps.time.minute());
        Serial.print(F(":"));
        Serial.println(gps.time.second());
      } else {
        Serial.println(F("INVALID"));
      }
      Serial.println();
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));


  if (sim.available() > 0) {
    String response = sim.readStringUntil('\n');
    Serial.println(response); 
    if (response.indexOf("LOCATION") != -1) {
      if (millis() - lastLocationCheck > locationCheckInterval) {
        lastLocationCheck = millis();
        delay(5000); // Delay 5 seconds
        if (gps.location.isValid()) {
          String googleMapsLink = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
          String message = "Temperature: " + String(temp.temperature) + " Celsius \nHumidity: " + String(humidity.relative_humidity) + "% RH" + "\nLocation: " + googleMapsLink;
          sendSMS(PHONE_NUMBER, message);
        } else {
          sendSMS(PHONE_NUMBER, "No GPS signal available.");
        }
      }
    }
  }
  
}


void sendSMS(String number, String message) {
  sim.println("AT+CPIN?");
  delay(100);
  if (sim.find("READY")) {
    sim.println("AT+CMGF=1");
    delay(100);
    sim.print("AT+CMGS=\"");
    sim.print(number);
    sim.println("\"");
    delay(100);
    sim.print(message);
    delay(100);
    sim.println((char)26);
    delay(100);
  } else {
    Serial.println("SIM card not ready");
  }
}

void RecieveMessage() {
  Serial.println("SIM800L Read an SMS");
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CNMI=1,2,0,0,0");
  delay(200);
  Serial.write("Unread Message done");
}
