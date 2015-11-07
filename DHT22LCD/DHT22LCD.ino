#include <SPI.h>
#include <DHT.h>
#include <LiquidCrystal595.h> 
#include <Wire.h>
#include "RTClib.h"
#include <SD.h>
 
// DHT Sensor Setup
#define DHTPIN 6
#define DHTTYPE DHT22

RTC_DS1307 rtc;
const int chipSelect = 10;
LiquidCrystal595 lcd(7,8,9); // datapin, latch, clock
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT object

void setup() {
    lcd.begin(16, 2); // 16 characters, 2 rows
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
    
    lcd.setCursor(0, 0);
    lcd.print("Initializing");
    lcd.setCursor(0, 1);
    lcd.print("SD Card...");
    delay(500);
   
    if (!SD.begin(chipSelect)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SD Card failed");
      while(1);
    }
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card Initialized.");
    delay(500);

    // if clock is not started, start it and set to compile time.
    if (! rtc.isrunning()) {
      lcd.setCursor(0, 0);
      lcd.print("RTC NOT running!");
      delay(500);
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    lcd.clear();
}

float tempf, humidity;

void loop() {
    DateTime now = rtc.now();
    
    humidity = dht.readHumidity();
    tempf = dht.readTemperature(true);
    
    char dTemp[33];
    char dHum[33];
    dTemp[32] = '\0';
    dHum[32] = '\0';
    
    fmtDouble(tempf, 2, dTemp, 32);
    fmtDouble(humidity, 2, dHum, 32);
    
    String timeStamp = "";
    
    timeStamp += String(now.year());
    timeStamp += String("-");
    timeStamp += String(now.month());
    timeStamp += String("-");
    timeStamp += String(now.day());
    timeStamp += " ";
    
    if (now.hour() < 10) { timeStamp += "0"; }
    timeStamp += String(now.hour());
    timeStamp += ":";
    
    if (now.minute() < 10) { timeStamp += "0"; }
    timeStamp += String(now.minute());
    timeStamp += ":";
    
    if (now.second() < 10) { timeStamp += "0"; }
    timeStamp += String(now.second());
    
    String dataLog = "";
    dataLog += timeStamp;
    dataLog += ",";
    dataLog += dTemp;
    dataLog += ",";
    dataLog += dHum;
    
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
    
    File dataFile = SD.open("tmplog.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.println(dataLog);
      dataFile.close();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("error opening");
      lcd.setCursor(0, 1);
      lcd.print("datalog.txt");
    }
    delay(60000);
}

void fmtDouble(double val, byte precision, char *buf, unsigned bufLen = 0xffff);
unsigned fmtUnsigned(unsigned long val, char *buf, unsigned bufLen = 0xffff, byte width = 0);

// Produce a formatted string in a buffer corresponding to the value provided.
// If the 'width' parameter is non-zero, the value will be padded with leading
// zeroes to achieve the specified width.  The number of characters added to
// the buffer (not including the null termination) is returned.
unsigned fmtUnsigned(unsigned long val, char *buf, unsigned bufLen, byte width)
{
 if (!buf || !bufLen)
   return(0);

 // produce the digit string (backwards in the digit buffer)
 char dbuf[10];
 unsigned idx = 0;
 while (idx < sizeof(dbuf))
 {
   dbuf[idx++] = (val % 10) + '0';
   if ((val /= 10) == 0)
     break;
 }

 // copy the optional leading zeroes and digits to the target buffer
 unsigned len = 0;
 byte padding = (width > idx) ? width - idx : 0;
 char c = '0';
 while ((--bufLen > 0) && (idx || padding))
 {
   if (padding)
     padding--;
   else
     c = dbuf[--idx];
   *buf++ = c;
   len++;
 }

 // add the null termination
 *buf = '\0';
 return(len);
}

//
// Format a floating point value with number of decimal places.
// The 'precision' parameter is a number from 0 to 6 indicating the desired decimal places.
// The 'buf' parameter points to a buffer to receive the formatted string.  This must be
// sufficiently large to contain the resulting string.  The buffer's length may be
// optionally specified.  If it is given, the maximum length of the generated string
// will be one less than the specified value.
//
// example: fmtDouble(3.1415, 2, buf); // produces 3.14 (two decimal places)
//
void fmtDouble(double val, byte precision, char *buf, unsigned bufLen)
{
 if (!buf || !bufLen)
   return;

 // limit the precision to the maximum allowed value
 const byte maxPrecision = 6;
 if (precision > maxPrecision)
   precision = maxPrecision;

 if (--bufLen > 0)
 {
   // check for a negative value
   if (val < 0.0)
   {
     val = -val;
     *buf = '-';
     bufLen--;
   }

   // compute the rounding factor and fractional multiplier
   double roundingFactor = 0.5;
   unsigned long mult = 1;
   for (byte i = 0; i < precision; i++)
   {
     roundingFactor /= 10.0;
     mult *= 10;
   }

   if (bufLen > 0)
   {
     // apply the rounding factor
     val += roundingFactor;

     // add the integral portion to the buffer
     unsigned len = fmtUnsigned((unsigned long)val, buf, bufLen);
     buf += len;
     bufLen -= len;
   }

   // handle the fractional portion
   if ((precision > 0) && (bufLen > 0))
   {
     *buf++ = '.';
     if (--bufLen > 0)
       buf += fmtUnsigned((unsigned long)((val - (unsigned long)val) * mult), buf, bufLen, precision);
   }
 }

 // null-terminate the string
 *buf = '\0';
}
