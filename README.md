# Arduino-NodeMCU-HomeAuto
A Home Automation system i designed and built for learning purposes.
I made it by hacking various bits of code and libraries together with my own code. 
The book "Electronics for Dummies" and the Arduino forum helped me build the circuit.

I programmed the Atmega chip as normal using the Arduino board and then removed it.
The circuitry on the left of the push button is for the 5V voltage regulator used 
by the Atmega. 

Using a SoftRTC and updating the time using NTP gives an accurate clock. 

The Atmega and NodeMCU communicate using I2C. Essentially the NodeMCU is 
the access point for the Atmega to connect to the Internet and get commands/report values to the MQTT server.
Using an MQTT client app for android allows you to control the relays and view sensor values remotely.
The relays can also be triggered by sensor events (i.e. change in temp/humidity) or by a clock time.

The LCD is controlled through a shift register to reduce the number of pins used.

![alt text](https://github.com/BobbyLeonard/Arduino-Espressif-HomeAuto/raw/master/Atmega%26NodeMCU.jpg)

![alt text](https://github.com/BobbyLeonard/Arduino-NodeMCU-HomeAuto/raw/master/InConstruction.jpg)


