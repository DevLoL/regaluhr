/*

 Clock using NTP on an ESP8266 show the time on a 7-segment display

 (C) 2017 Thomas R. Koll

 This code is in the public domain.

 */

#include <NTPClient.h> // https://github.com/arduino-libraries/NTPClient
#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino/
#include <WiFiUdp.h>

char ssid[] = "/dev/lol";
char password[] = "4dprinter";

#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#include <Timezone.h>    // https://github.com/JChristensen/Timezone
  
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60}; //Central European Standard Time
Timezone timezone(CEST, CET);

WiFiUDP ntpUDP;
// 0 minutes time offset, update every hour
// any timezone offset is dealt with by Timezone to get better precision
NTPClient timeClient(ntpUDP, "at.pool.ntp.org", 0, 60 * 60 * 1000);

Adafruit_7segment matrix = Adafruit_7segment();

void connectWifi() {
    // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void writeTime() {
  static time_t local_time;
  local_time = timezone.toLocal(timeClient.getEpochTime());
  // to make sure we have all the leading 0s
  matrix.writeDigitNum(0, hour(local_time) / 10 );
  matrix.writeDigitNum(1, hour(local_time) % 10 );
  matrix.writeDigitNum(3, minute(local_time) / 10 );
  matrix.writeDigitNum(4, minute(local_time) % 10 );
  matrix.drawColon(second(local_time) % 2);
  matrix.writeDisplay();
}

void setup() {
  Serial.begin(9600);
  matrix.begin(0x70); // 0x70 is the I2C address of the display
  matrix.setBrightness(10);

  connectWifi();

  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
  
  timeClient.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  timeClient.update();
  writeTime();
}
