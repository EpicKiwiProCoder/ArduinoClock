#include <FastLED.h>;
#include <SPI.h>;
#include <WiFiNINA.h>
#include <WiFiUdp.h>

int fps = 24;

int status = WL_IDLE_STATUS;
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP timestamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

const char ssid[] = "ABCM Not in Range";
const char pass[] = "marion013";
const int numLeds=90;
CRGB strip[numLeds];
int DATA_PIN = 9;
int cloc = 10;
int timeOffset = 2;
const int numWords = 22;
//woord words[6] = {"Logo:0102", "Uur:0407", "Twaalf:0813", "Elf:1518", "Tien:1921", "Negen:2225"};

unsigned long internetTime = 0;



unsigned long secondsLastUpdate = 0; 
class woord {
  public:
    String naam;
    int start;
    int eind;
    bool active = false;
    woord(String n, int b, int e) {
        this->naam = n;
        this->start = b;
        this->eind = e;
    }
    
};

woord words[numWords] = {woord("logo",0,2),woord("uur",3,6),woord("12u",7,12),woord("11u",14,17),woord("6u",18,19),woord("7u",20,24), woord("8u",25,27), 
woord("9u", 29, 32), woord("10u", 33, 35), woord("5u", 36, 38), woord("4u", 40, 42), woord("3u", 43, 46), woord("2u", 47, 50), woord("1u", 51, 53),
woord("15ov", 54, 57), woord("half", 59, 62), woord("vooruur", 63, 67), woord("overuur", 68, 71), woord("overdeel", 72, 75), woord("voordeel", 76,80),
woord("10ov", 81, 84), woord("5ov", 86, 89)};

CRGB getColor(int ledPosition) {
  int x = 10;
  int y = 10;
  
  if(ledPosition <= 17) {
    y = 0;
    x = 17 - ledPosition;
  } else if(ledPosition <= 35) {
    y = 1;
    x = ledPosition - 18;
  } else if(ledPosition <= 53) {
    y = 2;
    x = 36 - ledPosition;
  } else if(ledPosition <= 71) {
    y = 3;
    x = ledPosition - 54;
  } else if(ledPosition <= 89) {
    y = 4;
    x = 72 - ledPosition;
  } 
  int scale = 10;
  uint8_t nois = inoise8(x*scale-millis()/5,y*scale);
  float noise = (float) nois;
  CRGB color = CRGB(0,0,0);
  float value = 0;
  int maxi = 180;
  if (noise <= 64) {
    value = noise / 64;
   
    color = CRGB(maxi*value, 0, maxi*(1-value));
  } else if (noise <= 128) {
    value = (noise - 64) / 64;
    color = CRGB(maxi*(1-value), maxi*value, 0);
    
  } else if (noise <= 192) {
    value = (noise - 128) / 64;
    color = CRGB(0, maxi*(1 - value), maxi*value);
  } else {
    value = (noise - 191) / 64;
    color = CRGB(maxi * value, 0, maxi*(1-value));
  }
  
  return color;
  
}


// Zet de leds van de actieve woorden aan
void colorLeds() {
  for(int i=0; i<numWords;i++) {
    if(words[i].active) {
      for(int j = words[i].start; j<=words[i].eind; j++) {
        strip[j] = CRGB(255,200,255); // foregroundColor
      }  
    } else {
      for(int j = words[i].start; j<=words[i].eind; j++) {
        strip[j] = getColor(j); // backgroundColor
      } 
    }
  }
  FastLED.show();
}

void setWordActive(String n) {
  for(int i=0; i < numWords; i++) {
    if(words[i].naam == n) {
      words[i].active = true;  
    } 
  }  
}
void setWordInActive(String n) {
  for(int i=0; i < numWords; i++) {
    if(words[i].naam == n) {
      words[i].active = false;  
    } 
  }  
}
void resetAll() {
  for(int i=0; i < numWords; i++) {
    words[i].active = false;
  }  
}

// Internet tijd naar Uur:Min:Sec en set de leds
void SetWords() {
  unsigned long adjustedTime = internetTime + (millis() / 1000 - secondsLastUpdate);
  int hours = ((adjustedTime  % 86400L) / 3600) + timeOffset; // hours
  int OldMinutes = ((adjustedTime  % 3600) / 60); // minutes
  int seconds = (adjustedTime % 60); // seconds
  
  resetAll();
  int minutes = int(round(OldMinutes / 5.0))*5.0;
  if(hours > 12) {
    hours -= 12;
  }
  if(minutes == 60) {
    minutes = 0;
    hours++;
    if(hours == 13) {
      hours = 1;  
    }
  }

  // het is nu 12 uurs klok
  int nextHour = hours+1;
  if (nextHour == 13) {
    nextHour = 1;
  } 
  
  if(minutes == 0) {
    setWordActive(String(hours) + "u");
    setWordActive("uur");  
  } else if(minutes == 5 or minutes == 10 or minutes == 15) {
    setWordActive(String(minutes) + "ov");
    setWordActive("overuur");
    setWordActive(String(hours) + "u");
  } else if(minutes == 20 or minutes == 25) {
    setWordActive(String(30 - minutes) + "ov");
    setWordActive("voordeel");
    setWordActive("half");
    setWordActive(String(nextHour) + "u");
  } else if(minutes == 30) {
    setWordActive("half");
    setWordActive(String(nextHour) + "u");  
  } else if(minutes == 35 or minutes == 40) {
    setWordActive(String(minutes - 30) + "ov");
    setWordActive("overdeel");
    setWordActive("half");
    setWordActive(String(nextHour) + "u"); 
  } else if(minutes == 45 or minutes == 50 or minutes == 55) {
    setWordActive(String(60 - minutes) + "ov");
    setWordActive("vooruur");
    setWordActive(String(nextHour) + "u");  
  }
  colorLeds();
  //Serial.println("This :" + String(hours) + "   " + "next :" + String(nextHour));
  //Serial.println(String(hours) + " : " + String(minutes));
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  FastLED.addLeds<APA102, 9, 10, BGR>(strip, numLeds);
  FastLED.setBrightness(20); // van de 100
  FastLED.clear(); 
  setWordActive("logo");
  colorLeds();
  connectWifi();
  
}

void loop() {
  
  // Tijd van time server halen
  
  while(internetTime == 0) {
    resetAll();
    setWordActive("logo");
    colorLeds();
    unsigned long tempTime = getUtcTime();
    if (tempTime > 0){
      internetTime = tempTime; 
      secondsLastUpdate = millis() / 1000; 
      setWordInActive("logo");
      colorLeds();
    }
  }
  
  SetWords();
  
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500 / fps);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(500 / fps); 
  
  if (millis() / 1000 - secondsLastUpdate > 7200 or millis() / 1000 - secondsLastUpdate < -10) {
    internetTime = 0;
  }
}







//**** Connect WiFi ****
void connectWifi(){
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to WiFi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


// **** Tijd van timeserver ****
unsigned long getUtcTime(){
  
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
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
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
    return epoch;


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
  // wait ten seconds before asking for the time again
  delay(5000);
  return 0;
}




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
