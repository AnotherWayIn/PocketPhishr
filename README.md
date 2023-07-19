# PocketPhishr
An evil WiFi Access Point for your pocket - because

Code is based on the original here: https://github.com/adamff1/ESP8266-Captive-Portal, with some tweaks to work with a display. 


Here is the specific board the code is based on. If you choose to use a different board, you'll need to use the relevant settings and library for that display:
https://www.aliexpress.com/item/1005004513260449.html

There are some instructions included on setting up the board and U8g2 library within the Arduino IDE.
Unfortunately, I couldn't find this version without the pin headers, so you'll need to desolder/cut them to fit inside the case.

case you can 3d print for it here: https://www.thingiverse.com/thing:6099890






Once running, if you connect to the free wifi from a pc (probably easier than your phone) you can browse to the web server on http://172.0.0.1/creds to view all the saved credentials or to http://172.0.0.1/ssid to change the ssid.
The credentials are written to the eeprom so will persist if the device loses power.



![IMG_4487](https://github.com/AnotherWayIn/PocketPhishr/assets/10500665/691bf384-637d-4af8-b2d8-cbeb152cc0a4)

![Xnip2023-06-29_05-52-39](https://github.com/AnotherWayIn/PocketPhishr/assets/10500665/941055ce-3b88-497e-af2f-07562731b9de)
![IMG_4471 3](https://github.com/AnotherWayIn/PocketPhishr/assets/10500665/dee45064-4e52-4b82-98fc-5fe3bf8d1ffc)




