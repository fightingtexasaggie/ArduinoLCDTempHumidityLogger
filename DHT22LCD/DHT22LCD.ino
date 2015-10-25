#include <SPI.h>
#include <DHT.h>
#include <LiquidCrystal595.h> 
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;

LiquidCrystal595 lcd(7,8,9);     // datapin, latchpin, clockpin
                 
// DHT Sensor Setup
#define DHTPIN 6 // We have connected the DHT to Digital Pin 2
#define DHTTYPE DHT22 // This is the type of DHT Sensor (Change it to DHT11 if you're using that model)
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT object

void setup() {
    lcd.begin(16, 2);             // 16 characters, 2 rows
    lcd.noDisplay();
    lcd.clear();
    dht.begin();
    delay(1000);
    
    lcd.display();
    
    if (! rtc.begin()) {
      lcd.setCursor(0, 0);
      lcd.print("Couldn't find RTC");
    while (1);
  }
  
    if (! rtc.isrunning()) {
      lcd.setCursor(0, 0);
      lcd.print("RTC NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
}

float tempf, humidity;

void loop() {
    DateTime now = rtc.now();
    
    humidity = dht.readHumidity();
    tempf = dht.readTemperature(true);
    lcd.setCursor(0, 0);
    lcd.print(tempf);
    lcd.print("F");
    lcd.setCursor(8, 0);
    lcd.print(humidity);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute() < 10) { lcd.print("0"); }
    lcd.print(now.minute());
    lcd.print(":");
    if (now.second() < 10) { lcd.print("0"); }
    lcd.print(now.second());
    delay(5000);
}
