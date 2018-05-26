# Arduino-NodeMCU-HomeAuto
A Home Automation system i designed and built for learning purposes.
I made it by piecing together various bits of code and libraries 
from online with my own code. 
The book "Electronics for Dummies" and the Arduino forum helped me build the circuit.

I programmed the Atmega chip as normal using the Arduino board and then removed it.
The circuitry on the left of the push button is for the 5V voltage regulator used 
by the Atmega. 

The Atmega and NodeMCU communicate using I2C. Essentially the NodeMCU is 
the access point for the Atmega to connect to the Internet. Sending and receiving 
data from the MQTT server. Using an MQTT client app for android allows you to control the relays remotely.

The clock on the Atmega was running slow so the NodeMCU gets the NTP time 
and then the Atmega keeps its time by updating every few seconds. 

![alt text](https://github.com/BobbyLeonard/Arduino-Espressif-HomeAuto/raw/master/Atmega%26NodeMCU.jpg)
