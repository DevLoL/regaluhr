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

static time_t local_time;

Adafruit_7segment matrix = Adafruit_7segment();

#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS 60
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define DATA_PIN D6

// Define the array of leds
CRGB leds[NUM_LEDS];

#define LED_SPEED 10 // steps per second

void connectWifi() {
    // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void displayTimeOn7Segment() {
  // to make sure we have all the leading 0s
  matrix.writeDigitNum(0, hour(local_time) / 10 );
  matrix.writeDigitNum(1, hour(local_time) % 10 );
  matrix.writeDigitNum(3, minute(local_time) / 10 );
  matrix.writeDigitNum(4, minute(local_time) % 10 );
  matrix.drawColon(second(local_time) % 2);
  matrix.writeDisplay();
}

void setLEDcolor(int pos, CHSV color, int num_leds) {
  for (int i; i < num_leds; i++) {
    leds[(pos + i) % NUM_LEDS] = color;
  }
}
void displayTimeOnLEDs() {
  static int pos = 0;
  static int pos_20 = 20;
  static int pos_40 = 40;
  // show leds for second, minute and hour, send them around the strip
  setLEDcolor(pos, CHSV(second(local_time) * 4.25, 255, 255), 5);
  setLEDcolor(pos_40, CHSV(minute(local_time) * 4.25, 255, 255), 3);
  setLEDcolor(pos_20, CHSV(hour(local_time) * 10.625, 255, 255), 2);
  FastLED.show();

  // Only turn off the last LED, the rest was overwritten anyways
  leds[pos] = CRGB::Black;
  leds[pos_20] = CRGB::Black;
  leds[pos_40] = CRGB::Black;
  FastLED.show();

  pos = (pos + 1) % NUM_LEDS;
  pos_20 = (pos_20 + 1) % NUM_LEDS;
  pos_40 = (pos_40 + 1) % NUM_LEDS;
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  timeClient.update();
  local_time = timezone.toLocal(timeClient.getEpochTime());

  displayTimeOn7Segment();

  static unsigned long next_led_update = 0;
  if (next_led_update < millis()) {
    next_led_update = millis() + (LED_SPEED / 1000);
    displayTimeOnLEDs();
  }
}

void setup() {
  Serial.begin(9600);
  matrix.begin(0x70); // 0x70 is the I2C address of the display
  matrix.setBrightness(10);

  connectWifi();
  
  timeClient.begin();

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
}

