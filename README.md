# Arduino-Espressif-HomeAuto
A Home Automation system i put together myself for learning purposes.
I put it together by piecing various component parts of code and libraries 
from sources on the internet along with my own code

See Atmega&NodeMCU.jpg above for the wiring diagram.

I programmed the Atmega using the Arduino board and then removed it.
The circuitry on the left of the pushbutton is for the 5V voltage regulator used 
for the Atmega. 

The Atmega and NodeMCU communicate using I2C. Essentially the NodeMCU is 
the access point for the Atmega to connect to the Internet. Sending and receiving 
data from the MQTT server. Using an MQTT client app for android allows you to control the relays remotely.

The clock on the Atmega was running slow so the NodeMCU gets the NTP time 
and then the Atmega keeps its time by updating every few seconds. 

![alt text](https://github.com/BobbyLeonard/Arduino-Espressif-HomeAuto/raw/master/Atmega%26NodeMCU.jpg)
