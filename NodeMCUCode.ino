#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#define PIN_WIRE_SDA (0)
#define PIN_WIRE_SCL (2)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;
static const uint8_t D0   = 0;
static const uint8_t D2   = 2;

#include "../generic/common.h"

#endif /* Pins_Arduino_h */

#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "<SSID>"
#define WLAN_PASS       "<PASSWORD>"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "<yourAIOusername>"
#define AIO_KEY         "<youAIOkey>"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;

const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;

const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>

const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/Temperature";
const char HUMIDITY_FEED[] PROGMEM = AIO_USERNAME "/feeds/Humidity";

Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");


const char LEDSWITCH_FEED[] PROGMEM = AIO_USERNAME "/feeds/ledswitch";
const char FANSWITCH_FEED[] PROGMEM = AIO_USERNAME "/feeds/fanswitch";
const char LEDOR_FEED[] PROGMEM = AIO_USERNAME "feeds/ledoverride";
const char FANOR_FEED[] PROGMEM = AIO_USERNAME "feeds/fanoverride";

Adafruit_MQTT_Subscribe LEDbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ledswitch");
Adafruit_MQTT_Subscribe Fanbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/fanswitch");
Adafruit_MQTT_Subscribe LEDor = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ledoverride");
Adafruit_MQTT_Subscribe Fanor = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/fanoverride");

void MQTT_connect();

String message = "...";

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

void setup() {
  Wire.begin(D0, D2);
  Serial.begin(115200);
  delay(10);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscriptions for feeds.

  mqtt.subscribe(&LEDbutton);
  mqtt.subscribe(&Fanbutton);
  mqtt.subscribe(&LEDor);
  mqtt.subscribe(&Fanor);

  Udp.begin(localPort);
}

void loop() {

  char buffer[32];
  message.toCharArray(buffer, 32);
  Wire.beginTransmission(8); /* begin with device address 8 */
  Wire.write(buffer);  /* sends hello string */
  Wire.endTransmission();    /* stop transmitting */
  message = "...";

  Wire.requestFrom(8, 15); /* request & read data of size 15 from slave */
  String cache = "";

  while (Wire.available()) {
    char c = Wire.read();
    cache += c;
  }

  String Tempbuf = "";
  for (int T = 2; T <= 6; T++) {
    Tempbuf.concat(cache[T]);
  }

  String Humdbuf = "";
  for (int H = 10; H <= 14; H++) {
    Humdbuf.concat(cache[H]);
  }

  float Temp = Tempbuf.toFloat();
  float Humd = Humdbuf.toFloat();
  
  sendNTPpacket(timeServer);
  
  String TimeBuffer = "";
  
  delay(1000);
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // UTC is the time at Greenwich Meridian (GMT)
    // print the hour (86400 equals secs per day)
    TimeBuffer +=  "UTC";
    TimeBuffer +=  epoch;
  
  Serial.println(epoch);
  char NTPbuffer[32];
  TimeBuffer.toCharArray(NTPbuffer, 32);
  
  Wire.beginTransmission(8); /* begin with device address 8 */
  Wire.write(NTPbuffer);  /* sends hello string */
  Wire.endTransmission();    /* stop transmitting */
  delay(100);
  


  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &LEDbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)LEDbutton.lastread);

      if (strcmp((char*)LEDbutton.lastread, "On") == 0) {
        Serial.println("Running On Function");
        message = "LEDON";
      }
      if (strcmp((char*)LEDbutton.lastread, "Off") == 0) {
        Serial.println("Running Off Function");
        message = "LEDOFF";
      }
    }

    if (subscription == &Fanbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)Fanbutton.lastread);

      if (strcmp((char*)Fanbutton.lastread, "On") == 0) {
        Serial.println("Running On Function");
        message = "FANON";
      }
      if (strcmp((char*)Fanbutton.lastread, "Off") == 0) {
        Serial.println("Running Off Function");
        message = "FANOFF";
      }
    }

    if (subscription == &LEDor) {
      Serial.print(F("Got: "));
      Serial.println((char *)LEDor.lastread);

      if (strcmp((char*)LEDor.lastread, "On") == 0) {
        Serial.println("Running ORR Function");
        message = "LEDOR";
      }
      if (strcmp((char*)LEDor.lastread, "Off") == 0) {
        Serial.println("NOT Running ORR Function");
        message = "NOTLEDOR";
      }
    }

    if (subscription == &Fanor) {
      Serial.print(F("Got: "));
      Serial.println((char *)Fanor.lastread);

      if (strcmp((char*)Fanor.lastread, "On") == 0) {
        Serial.println("Running ORR Function");
        message = "FANOR";
      }
      if (strcmp((char*)Fanor.lastread, "Off") == 0) {
        Serial.println("NOT Running ORR Function");
        message = "NOTFANOR";
      }
    }
  }



  // Now we can publish stuff!
  Serial.println(F("Sending values ... "));
  Serial.print("Temp : ");
  Serial.println(Temp);
  Serial.print("Humd : ");
  Serial.println(Humd);
  Serial.print("...");
  if (! Temperature.publish(Temp) || ! Humidity.publish(Humd)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!\n"));
  }
}
}

// ping the server to keep the mqtt connection alive
// NOT required if you are publishing once every KEEPALIVE seconds
/*
  if(! mqtt.ping()) {
  mqtt.disconnect();
  }
*/


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}
}
