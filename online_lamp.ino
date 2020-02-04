#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <BlynkSimpleEsp8266.h>

#define LED_PIN    8  //Pin number on ESP8266 board
#define NUM_LEDS    3 //Number of LEDs 
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

//Webserver declaration
ESP8266WebServer server(80);

//WiFi Setup variables
char ssidc[30]; //Stores the router name for IOT
char passwordc[30]; //Stores the password for IOT

const char auth[] = "AUTH TOKEN"; //Make acount on Blynk.com and get your AUTH TOKEN  

IPAddress wap_ip(192, 168, 1, 1); //Server ip adres when in WAP mode
IPAddress wap_gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);
const char *ssid2 = "ESP8266_Hotspot";
const char *password2 = "onlineLamp123";

#define button_Pin 16
String ssid_Arg;// Error message for ssid input
String password_Arg;// Error message for password input
String INDEX_HTML;

bool all_leds, change, flg, just_led1, just_led2, just_led3, wap_flg, iot_flg;
int redPin, greenPin, bluePin, update_speed, fade, isr_br = 0, t = 0, led1, led2, led3;


//Reads a string out of EEPROM memory
String read_string(int l, int p) {
  String temp;
  for (int n = p; n < l + p; ++n)
  {
    if (char(EEPROM.read(n)) != ';') {
      temp += String(char(EEPROM.read(n)));

    } else n = l + p;

  }
  return temp;
}

///////////////////Blynk variables////////////////////////////

BLYNK_WRITE(V9 ) {
  just_led3 = param.asInt();

}

BLYNK_WRITE(V8) {
  just_led2 = param.asInt();

}

BLYNK_WRITE(V7) {
  just_led1 = param.asInt();

}

BLYNK_WRITE(V6) {
  update_speed = param.asInt();

}

BLYNK_WRITE(V5) {
  fade = param.asInt();
}

BLYNK_WRITE(V4) {
  int brightness = param.asInt();
  FastLED.setBrightness(brightness);
  FastLED.show();
}

BLYNK_WRITE(V3) {
  all_leds = param.asInt();
  change = 1;
}

BLYNK_WRITE(V0) {
  redPin = param.asInt();
  change = 1;
}



BLYNK_WRITE(V1) {
  greenPin = param.asInt();
  change = 1;
}



BLYNK_WRITE(V2) {
  bluePin = param.asInt();
  change = 1;
}

///////////////////////////////////////////////////////////////


BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}




void setup() {

  EEPROM.begin(512);
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  pinMode(button_Pin, INPUT);
  leds[0].setRGB(0, 0, 0);
  leds[1].setRGB(0, 0, 0);
  leds[2].setRGB(0, 0, 0);
  FastLED.show();

  if (digitalRead(button_Pin) == LOW) {
    IOT();
  } else if (digitalRead(button_Pin) == HIGH) {
    WAP();
  }

  //Configuring Web Server
  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

}


void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


void WAP() {
  delay(1000);
  WiFi.softAPConfig(wap_ip, wap_gateway, subnet);
  Serial.print("Setting soft-AP ... ");
  WiFi.softAP(ssid2, password2);
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  wap_flg = 1;
}


void IOT() {

  //reading the ssid and password out of memory
  delay(3000);
  Blynk.begin(auth, ssidc, passwordc);
  delay(1000);
  String string_Ssid = "";
  String string_Password = "";
  string_Ssid = read_string(30, 0);
  string_Password = read_string(30, 100);
  Serial.println("ssid: " + string_Ssid);
  Serial.println("Password:" + string_Password);
  string_Password.toCharArray(passwordc, 30);
  string_Ssid.toCharArray(ssidc, 30);


  Serial.println("Connected");

  iot_flg = 1;
}

void loop() {
  if (wap_flg) {
    server.handleClient();
    redFade();
  }


  if (iot_flg) {

    Blynk.run();

    if (!all_leds && change && fade != 1 && !just_led1 && !just_led2 && !just_led3) {

      allLEDs(redPin, greenPin, bluePin);
      change = 0;

    }

    if (just_led1 && change && !just_led2 && !just_led3) {

      leds[0].setRGB(redPin, greenPin, bluePin);
      change = 0;
      FastLED.show();
    }


    if (just_led2 && change && just_led1 && !just_led3) {


      leds[1].setRGB(redPin, greenPin, bluePin);
      change = 0;
      FastLED.show();
    }


    if (just_led3 && change && just_led2 && just_led1) {

      leds[2].setRGB(redPin, greenPin, bluePin);
      change = 0;
      FastLED.show();
    }


    if (all_leds && fade != 1) {

      leds[0].setRGB(255, 0, 0);
      leds[1].setRGB(0, 255, 0);
      leds[2].setRGB(0, 0, 255);
      FastLED.show();
      change = 0;

    }


    if (fade == 1) {
      rainbow();
    }

  }

}
void allLEDs(int x, int y, int z) {
  leds[0].setRGB(x, y, z);
  leds[1].setRGB(x, y, z);
  leds[2].setRGB(x, y, z);
  FastLED.show();
}

// handles htttp request for path /

void handleRoot() {
  if (digitalRead(button_Pin) == HIGH) {
    String responce = "<H2>Your Are Connected to demo Internet Of Things Device</H2>";
    responce += "<p>Here are instructions on how to turn the onboard led on and off<br>";
    responce += "<p>To Turn on the onboard LED type http://";
    responce += String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]);
    responce += "/on in your browser<br>";
    responce += "<p>To Turn off the onboard LED type http://";
    responce += String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]);
    responce += "/off in your browser";
    server.send(200, "text/html", responce);
  } else {
    create_form("", "");
    server.send(200, "text/html", INDEX_HTML);
  }
}


// This function creates main input form
void create_form(String ssidr, String passwordr) {
  INDEX_HTML = "<!DOCTYPE HTML>";
  INDEX_HTML += "<html>";
  INDEX_HTML += "<head>";
  INDEX_HTML += "<meta content=\"text/html; charset=ISO-8859-1\"";
  INDEX_HTML += " http-equiv=\"content-type\">";
  INDEX_HTML += "<title>ESP8266 Web Form</title>";
  INDEX_HTML += "</head>";
  INDEX_HTML += "<body>";
  INDEX_HTML += "<h1>ESP8266 Web Form</h1>";
  INDEX_HTML += "<FORM action=\"/submit\" method=\"post\">";
  INDEX_HTML += "<P>";
  INDEX_HTML += ssid_Arg;
  INDEX_HTML += "<br><label>ssid:&nbsp;</label>";
  INDEX_HTML += "<input size=\"30\" maxlength=\"30\" value=\"" ;
  INDEX_HTML += ssidr;
  INDEX_HTML += "\" name=\"ssid\">";
  INDEX_HTML += "<br>";
  INDEX_HTML += password_Arg;
  INDEX_HTML += "<br><label>Password:&nbsp;</label><input size=\"30\" maxlength=\"30\"  value=\"";
  INDEX_HTML += passwordr;
  INDEX_HTML += "\"name=\"Password\">";
  INDEX_HTML += "<br>";
  INDEX_HTML += "<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">";
  INDEX_HTML += "</P>";
  INDEX_HTML += "</FORM>";
  INDEX_HTML += "</body>";
  INDEX_HTML += "</html>";
}
//Handles  http request for path /submit
void handleSubmit() {
  int error[2];
  int err = 0;
  if (digitalRead(button_Pin) == LOW) {
    if (server.hasArg("ssid") && server.hasArg("Password")) { //If all form fields contain data call handelSubmit()
      error[0] = ssid_error_Check(server.arg("ssid"));
      error[1] = password_error_Check(server.arg("Password"));
      for (int n = 0; n < 2; n++) {
        if (error[n] != 0) {
          create_form(server.arg("ssid"), server.arg("Password"));
          server.send(200, "text/html", INDEX_HTML);
          err = 1;
          break;
        }
      }
      if (err == 0) {
        String response = "<h1>This is the information you entered </h1><BR>";
        response += "<p>The ssid is ";
        response += server.arg("ssid");
        response += "<br>";
        response += "And the password is ";
        response += server.arg("Password");
        response += "<br>";

        response += "</P><BR>";
        response += "<P>Reboot the ESP8266 module and enjoy your LED lamp!<br>";
        response += "</P>";
        response += "<H2><a href=\"/\">go home</a></H2><br>";
        server.send(200, "text/html", response);
        //calling function that writes data to memory
        write_to_Memory(String(server.arg("ssid")), String(server.arg("Password")));
        greenFade();
      }
    } else {
      create_form("", "");
      server.send(200, "text/html", INDEX_HTML);
    }

  } else { //button check
    server.send(200, "text/html", "<H1>check the position of the setup button and reset ESP8266</h1>");
  }

}
//Checks for correct construction of the SSID
int ssid_error_Check(String content) {
  int error = 0;
  if (content.length() < 2 || content.length() > 30) {
    ssid_Arg = "Your SSID can't be smaller than 2 characters, and not bigger then 30";
    error = 1;
  } else if (content.indexOf(';') != -1) {
    ssid_Arg = "The SSID can't contain ;";
    error = 1;
  } else {
    ssid_Arg = "";
    error = 0;
  }
  return error;
}
//Checks for the correct construction of the password
int password_error_Check(String content) {
  int error = 0;
  if (content.length() < 8 || content.length() > 30) {
    password_Arg = "Your password can't be smaller than 8 characters, and not bigger then 30";
    error = 1;
  } else if (content.indexOf(';') != -1) {
    password_Arg = "The password can't contain ;";
    error = 1;
  } else if (content.indexOf('"') != -1) {
    password_Arg = "The password can't contain \"";
    error = 1;
  } else if (content.indexOf(';') != -1) {
    password_Arg = "The password can't contain \'";
    error = 1;
  } else {
    password_Arg = "";
    error = 0;
  }
  return error;
}

void write_to_Memory(String s, String p) {
  s += ";";
  write_EEPROM(s, 0);
  p += ";";
  write_EEPROM(p, 100);
  EEPROM.commit();
}
//write to memory
void write_EEPROM(String x, int pos) {
  for (int n = pos; n < x.length() + pos; n++) {
    EEPROM.write(n, x[n - pos]);
  }
}



void rainbow() {

  int i = 0;
  int j = 0;
  int k = 0;
  int l = 0;
  int m = 0;
  int n = 0;
  while (fade == 1 && i < 254 * 2) {
    allLEDs(255, i / 2, 0);
    delay(update_speed);
    Blynk.run();

    i++;
  }

  while (fade == 1 && j < 254 * 2) {
    allLEDs(255 - j / 2, 255, 0);
    delay(update_speed);
    Blynk.run();

    j++;
  }


  while (fade == 1 && k < 254 * 2) {
    allLEDs(0, 255, k / 2);
    delay(update_speed);
    Blynk.run();

    k++;
  }


  while (fade == 1 && l < 254 * 2) {
    allLEDs(0, 255 - l / 2, 255);
    delay(update_speed);
    Blynk.run();

    l++;
  }


  while (fade == 1 && m < 254 * 2) {
    allLEDs(m / 2, 0, 255);
    delay(update_speed);
    Blynk.run();

    m++;
  }


  while (fade == 1 && n < 254 * 2) {
    allLEDs(255, 0, 255 - n / 2);
    delay(update_speed);
    Blynk.run();

    n++;
  }


}

void greenFade() {
  while (1) {
    for (int i = 0; i < 255 ; i++) {
      allLEDs(0, i, 0);
      delay(10);
    }


    for (int j = 255; j > 1 ; j--) {
      allLEDs(0, j, 0);
      delay(10);
    }


  }

}

void redFade() {
  for (int i = 0; i < 255 ; i++) {
    allLEDs(i, 0, 0);
    delay(10);
  }


  for (int j = 255; j > 1 ; j--) {
    allLEDs(j, 0, 0);
    delay(10);
  }

}
