This simple, low cost, accurate, IoT sensor is designed to measure solution PH levels in a reservoir. This device has the following features:
-	Easy calibration based on reference solutions of PH 4 and 7
-	Headless Wi-Fi setup allowing for connection to home Wi-Fi network or standalone Wi-Fi broadcast.
-	The option to post data to an API endpoint at a configurable frequency (in minutes)
-	Simple 3d printed enclosure (STL still under dev but included)
-	OTA support for sending updated complied binaries

Hardware needed (under $60):
-	DFRobot Analog PH Sensor: https://www.dfrobot.com/product-1782.html
-	ESP8266 NodeMCU: https://www.amazon.com/HiLetgo-Internet-Development-Wireless-Micropython/dp/B010O1G1ES/ref=sr_1_7_sspa?crid=2MUXP747Y3CPV&keywords=node%2Bmcu&qid=1697305414&sprefix=node%2Bmcu%2Caps%2C85&sr=8-7-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9tdGY&th=1

Wiring: 
(https://github.com/Techn0Hippie/ABMeter_PH/blob/main/ABdc_PH_wire.jpg)

Setup:
-	Download and install Arduino if you do not already have it: https://www.arduino.cc/en/software
-	From Tools > Board > Select “Generic ESP8266 Module”
-	Open ABMeter_PH2.ino and send it to the board connected over USB 
-	One device is flashed, disconnect power, hook up the PH meter and power on the device.
-	Device will broadcast its own WiFI with the following creds: 
    SSID: Autobud_DataCollector
    Password: Grow4life
-	Once you connect to that network browse to http://192.168.4.1
-	Refer to the UI image to see where to enter your creds, or continue without adding the device to your WiFi Network

Configuration:
The PH meter must be calibrated before use. The DFRobot Analog PH Sensor comes with PH4 and PH7 reference solutions. To calibrate, dip the sensor in the respective
Solution and press calibrate on the UI for the solution type.

End notes: This device is still in testing but thus far it has been accurate to 0.1 PH to my Blue Labs PH meter. Calibration is key 
