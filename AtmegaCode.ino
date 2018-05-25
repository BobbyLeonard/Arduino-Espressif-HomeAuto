#include <Time.h>
#include <TimeLib.h>
#include <Arduino.h>
#include "RTClib.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ShiftedLCD.h>
#include <SPI.h>

#define DHTPIN            7
#define DHTTYPE           DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);
char TempBuf[8];
char HumdBuf[8];
bool LEDState;
bool FanState;
bool TempOR;
bool HumdOR;
LiquidCrystal lcd(10);
RTC_Millis rtc;
const uint8_t SPRINTF_BUFFER_SIZE = 32;
char inputBuffer[SPRINTF_BUFFER_SIZE];
bool NTPBegin = false;


void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
  Serial.begin(115200);           /* start serial for debug */
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  lcd.begin(16, 2);
  while (!NTPBegin) {
    delay(10);
  }
}

void loop() {
  uint16_t tokens, yr, mh, dy, hr, me, sd;
  bool flag = false;
  if (!flag) {
    tokens = sscanf(inputBuffer, "%*s %d-%d-%d %d:%d:%d;",                // Use sscanf() to parse the date/  //
                    &yr, &mh, &dy, &hr, &me, &sd);                        // time into variables              //
    flag = true;
  }
  else {
    tokens = sscanf(inputBuffer, "%*s %d:%d:%d;",                         // Use sscanf() to parse the date/  //
                    &hr, &me, &sd);                                       // time into variables              //
  }
  static uint8_t secs;                                                    // store the seconds value          //
  if (secs != second()) {                                                 // Output if seconds have changed   //
    sprintf(inputBuffer, "%04d-%02d-%02d %02d:%02d:%02d", year(),         // Use sprintf() to pretty print    //
            month(), day(), hour(), minute(), second());                  // date/time with leading zeros     //
    Serial.println(inputBuffer);                                          // Display the current date/time    //
    secs =  second();                                                     // Set the counter variable         //
  } 

  sensors_event_t event;
  dht.temperature().getEvent(&event);

  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
    lcd.setCursor(0, 0);
    lcd.print("Error reading temperature!");

  }

  else {
    lcd.setCursor(0, 0);
    lcd.print("Temp:   ");
    lcd.print(event.temperature);
    lcd.print(" *C");

    if (!TempOR) {
      if (event.temperature >= 20) {
        digitalWrite(6, HIGH);
      }
    }

    else {
      if (LEDState) {
        digitalWrite(6, HIGH);
      }
    }
  }

  // Get humidity event and print its value.
  sensors_event_t eventH;
  dht.humidity().getEvent(&eventH);

  if (isnan(eventH.relative_humidity)) {
    Serial.println("Error reading humidity!");
    lcd.setCursor(0, 1);
    lcd.print("Error reading humidity!");
  }

  else {
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(eventH.relative_humidity);
    lcd.print("%");
    if (!HumdOR) {
      if (eventH.relative_humidity >= 50) {
        digitalWrite(5, HIGH);
      }
    }

    else {
      if (FanState) {
        digitalWrite(5, HIGH);
      }
    }
  }

  dtostrf(event.temperature, 6, 2, TempBuf);
  dtostrf(eventH.relative_humidity, 6, 2, HumdBuf);

}

// function that executes whenever data is received from master
void receiveEvent(int howMany) {
  NTPBegin = true;
  String cache = "";
  String epoch = "";
  String TimeBuffer = "";
  uint16_t yr, mh, dy, hr, me, sd;
  while (0 < Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
    cache += c;
  }

  if (cache.startsWith("UTC", 0)) {
    Serial.print("Cache length: ");
    Serial.println(cache.length());
    for (int T = 3; T <= cache.length(); T++) {
      epoch.concat(cache[T]);

    }
    //Serial.print("The time is ");
    //Serial.println(epoch);
    TimeBuffer += (epoch.toInt()  % 86400L) / 3600;
    TimeBuffer += ":";
    if (((epoch.toInt() % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      TimeBuffer += "0";
    }
    TimeBuffer += (epoch.toInt()  % 3600) / 60;
    TimeBuffer += ":";
    if ((epoch.toInt() % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      TimeBuffer += "0";
    }
    TimeBuffer += epoch.toInt() % 60;
    char outbuffer[32];
    TimeBuffer.toCharArray(outbuffer, 32);
    for ( int i = 0 ; i <= TimeBuffer.length() ; ++i ) {
      inputBuffer[ i ] = outbuffer[ i ];
    }
    Serial.print("NTP Time : ");
    Serial.println(TimeBuffer);
    //Serial.println(TimeBuffer.length());
    time_t t = epoch.toInt();
    yr = year(t);
    mh = month(t);
    dy = day(t);
    hr = (epoch.toInt()  % 86400L) / 3600;
    me = (epoch.toInt()  % 3600) / 60;
    sd = epoch.toInt() % 60;
    rtc.begin(DateTime(yr, mh, dy, hr, me, sd));
    rtc.adjust(DateTime(yr, mh, dy, hr, me, sd));     // Adjust the RTC date/time         //
    Serial.print(F("Date has been set.\n"));
    cache = "";

  }

  char buffer[32];
  cache.toCharArray(buffer, 32);

  if (strcmp(buffer, "LEDOR") == 0) {
    Serial.println("LED OR");
    TempOR = true;
  }

  if (strcmp(buffer, "NOTLEDOR") == 0) {
    Serial.println("LED OR OFF");
    TempOR = false;
  }

  if (strcmp(buffer, "FANOR") == 0) {
    Serial.println("FAN OR");
    HumdOR = true;
  }

  if (strcmp(buffer, "NOTFANOR") == 0) {
    Serial.println("FAN OR OFF");
    HumdOR = false;
  }

  if (strcmp(buffer, "LEDON") == 0) {
    Serial.println("Turn on LED");
    digitalWrite(6, HIGH);
    LEDState = true;
  }
  if (strcmp(buffer, "LEDOFF") == 0) {
    Serial.print("Turn off LED");
    digitalWrite(6, LOW);
    LEDState = false;
  }
  if (strcmp(buffer, "FANON") == 0) {
    Serial.println("Turn on Fan");
    digitalWrite(5, HIGH);
    FanState = true;
  }
  if (strcmp(buffer, "FANOFF") == 0) {
    Serial.print("Turn off Fan");
    digitalWrite(5, LOW);
    FanState = false;
  }
}



// function that executes whenever data is requested from master
void requestEvent() {
  Wire.write("T");
  Wire.write(TempBuf);  /*send string on request */
  Wire.write(" H");
  Wire.write(HumdBuf);
}

void readCommand() {                                                          //                                  //
  static uint8_t inputBytes = 0;                                              // Variable for buffer position     //
  while (Serial.available()) {                                                // Loop while incoming serial data  //
    inputBuffer[inputBytes] = Serial.read();                                  // Get the next byte of data        //
    if (inputBuffer[inputBytes] != '\n' && inputBytes < SPRINTF_BUFFER_SIZE)  // keep on reading until a newline  //
      inputBytes++;                                                           // shows up or the buffer is full   //
    else {                                                                    //                                  //
      inputBuffer[inputBytes] = 0;                                            // Add the termination character    //
      for (uint8_t i = 0; i < inputBytes; i++)                                // Convert the whole input buffer   //
        inputBuffer[i] = toupper(inputBuffer[i]);                             // to uppercase characters          //
      Serial.print(F("\nCommand \""));                                        //                                  //
      Serial.write(inputBuffer);                                              //                                  //
      Serial.print(F("\" received.\n"));                                      //                                  //
      enum commands { SetDate, Unknown_Command };                             // of commands enumerated type      //
      commands command;                                                       // declare enumerated type          //
      char workBuffer[10];                                                    // Buffer to hold string compare    //
      sscanf(inputBuffer, "%s %*", workBuffer);                               // Parse the string for first word  //
      if      (!strcmp(workBuffer, "SETDATE" )) command = SetDate;            // Set command number when found    //
      else command = Unknown_Command;                                         // Otherwise set to not found       //
      uint16_t tokens, year, month, day, hour, minute, second;                // Variables to hold parsed dt/tm   //
      switch (command) {                                                      // Action depending upon command    //
        /***********************************************************************************************************
        ** Set the device time and date                                                                           **
        ***********************************************************************************************************/
        case SetDate:                                                         // Set the RTC date/time            //
          tokens = sscanf(inputBuffer, "%*s %d-%d-%d %d:%d:%d;",              // Use sscanf() to parse the date/  //
                          &year, &month, &day, &hour, &minute, &second);      // time into variables              //
          if (tokens != 6)                                                    // Check to see if it was parsed    //
            Serial.print(F("Unable to parse date/time\n"));                   //                                  //
          else {                                                              //                                  //
            rtc.adjust(DateTime(year, month, day, hour, minute, second));     // Adjust the RTC date/time         //
            Serial.print(F("Date has been set."));                            //                                  //
          } // of if-then-else the date could be parsed                       //                                  //
          break;  //                                  //

        case Unknown_Command:                                                 // Show options on bad command      //
        default:                                                              //                                  //
          Serial.println(F("Unknown command. Valid commands are:"));          //                                  //
          Serial.println(F("SETDATE yyyy-mm-dd hh:mm:ss"));                   //                                  //
      } // of switch statement to execute commands                            //                                  //
      inputBytes = 0; // reset the counter                                    //                                  //
    } 
  } 
} 

