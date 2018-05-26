# Arduino-NodeMCU-HomeAuto
A Home Automation system i designed and built for learning purposes.
I made it by piecing together various bits of code and libraries 
from sources on the internet with my own code. 
"Electronics for Dummies" helped me build the circuit.

See below for the wiring diagram.

I programmed the Atmega chip over usb using the Arduino board and then removed it.
The circuitry on the left of the pushbutton is for the 5V voltage regulator used 
for the Atmega. 

The Atmega and NodeMCU communicate using I2C. Essentially the NodeMCU is 
the access point for the Atmega to connect to the Internet. Sending and receiving 
data from the MQTT server. Using an MQTT client app for android allows you to control the relays remotely.

The clock on the Atmega was running slow so the NodeMCU gets the NTP time 
and then the Atmega keeps its time by updating every few seconds. 

![alt text](https://github.com/BobbyLeonard/Arduino-Espressif-HomeAuto/raw/master/Atmega%26NodeMCU.jpg)
