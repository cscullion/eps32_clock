#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "secrets.h"

// WiFi network name and password:
const char *ssid = WIFI_SSI;
const char *wifipw = WIFI_PASSWORD;

int latchPin = 22;			// 74HC595 STCP (pin 12)
int clockPin = 21;			// 74HC595 SHCP (pin 11)
int dataPin = 23;			// 74HC595 DS   (pin 14)
const int LED_PIN = 2;		// lights up when connecting to wifi

byte digits[4];				// holds the digits to display

// 7-segment display LED names

#define LED_DP 0b10000000
#define LED_A  0b01000000
#define LED_B  0b00100000
#define LED_C  0b00010000
#define LED_D  0b00001000
#define LED_E  0b00000100
#define LED_F  0b00000010
#define LED_G  0b00000001

// LED patterns for numerical digits

byte pattern[10] = {
  (byte)~(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F         ),    // 0
  (byte)~(        LED_B | LED_C                                 ),    // 1
  (byte)~(LED_A | LED_B |         LED_D | LED_E |         LED_G ),    // 2
  (byte)~(LED_A | LED_B | LED_C | LED_D |                 LED_G ),    // 3
  (byte)~(        LED_B | LED_C |                 LED_F | LED_G ),    // 4
  (byte)~(LED_A |         LED_C | LED_D |         LED_F | LED_G ),    // 5
  (byte)~(LED_A |         LED_C | LED_D | LED_E | LED_F | LED_G ),    // 6
  (byte)~(LED_A | LED_B | LED_C                                 ),    // 7
  (byte)~(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G ),    // 8
  (byte)~(LED_A | LED_B | LED_C | LED_D |         LED_F | LED_G )     // 9
};


void setTimezone(String timezone){
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

void initTime(String timezone){
  struct tm timeinfo;

  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  if(!getLocalTime(&timeinfo)){
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time 1");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
}

void  startWifi(){
	digitalWrite(LED_PIN, HIGH); // LED on
	WiFi.begin(ssid, wifipw);
	Serial.println("Connecting Wifi");
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.print("Wifi RSSI=");
	Serial.println(WiFi.RSSI());
	digitalWrite(LED_PIN, LOW); // LED off
}

void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst){
  struct tm tm;

  tm.tm_year = yr - 1900;   // Set date
  tm.tm_mon = month-1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;      // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}

byte setPattern(int c, bool dp) {
  byte result = 0xff;

  if ((c >= 0) && (c <= 9)) {
    result = pattern[c];
  }
  if (dp) {
    result = ~result;
    result = result | LED_DP;
    result = ~result;
  }
  return(result);
}

void setup() {
	Serial.begin(115200);
	Serial.setTimeout(10000);

	// Set all the pins of 74HC595 as OUTPUT
	pinMode(latchPin, OUTPUT);
	pinMode(dataPin, OUTPUT);  
	pinMode(clockPin, OUTPUT);

	pinMode(LED_PIN, OUTPUT);

  // Connect to the WiFi network
  
	startWifi();

	initTime("EST5EDT,M3.2.0,M11.1.0");   // Set for New_York. see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
	printLocalTime();
}

// updateShiftRegister() - This function sets the latchPin to low, then calls the Arduino function 'shiftOut'
// to shift out contents of variable 'leds' in the shift register before putting the 'latchPin' high again.

void updateShiftRegister(byte d) {
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, d);
   digitalWrite(latchPin, HIGH);
}

void loop() {

	struct tm timeinfo;
	int lHour = 0;
	int lMin = 0;

	if (WiFi.status() != WL_CONNECTED) {
		startWifi();
	}

	printLocalTime();
	getLocalTime(&timeinfo);
	lHour = timeinfo.tm_hour;
	if (lHour > 12) {
		lHour = lHour - 12;
	}
	lMin = timeinfo.tm_min;

	digits[3] = setPattern(lMin % 10, false);
	digits[2] = setPattern(lMin / 10, false);
	digits[1] = setPattern(lHour % 10, true);
	digits[0] = setPattern((lHour < 10) ? -1 : 1, false);
	for (int i=3; i>= 0; i--) {
		updateShiftRegister(digits[i]);
	}

  delay(2000);
 }
