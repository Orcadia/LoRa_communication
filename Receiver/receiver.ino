#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"
#include <Arduino.h>
#include <string.h>
#include <WiFi.h>
#include <ThingSpeak.h>
 
SSD1306  display(0x3c, 4, 15);
 
//OLED pins to ESP32 GPIOs via this connection:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16
// WIFI_LoRa_32 ports
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
 
#define SS      18
#define RST     14
#define DI0     26
#define BAND    868E6
 
const char* ssid = "YOUR_SSID";
const char* password = "PASSWD";
const char* key = "API_WRITE_KEY";
const int chan_id = CHANNEL_ID;
WiFiClient client;

void setup() {
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
   
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  while (!Serial); //if just the the basic function, must connect to a computer
  delay(1000);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("LoRa Receiver"); 
  display.drawString(5,5,"LoRa Receiver"); 
  display.display();
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
   
  if (!LoRa.begin(BAND)) {
    display.drawString(5,25,"Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");
  display.drawString(5,25,"LoRa Initializing OK!");
  display.display();

  ThingSpeak.begin(client);  // Initialize ThingSpeak
}
void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packets
    Serial.print("Received packet. ");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    
    
    display.display();
    // read packet
    while (LoRa.available()) {
      String data         = LoRa.readString();
      String temperature  = "";
      String pressure     = "";

      if(data.startsWith("nNg0Twgzs5q",0)){
        data.replace("nNg0Twgzs5q","");

        int delimiterIndex  = data.indexOf(",");
        //display.drawString(10,22, String(delimiterIndex)); // Debugging
        if (delimiterIndex != -1) {
          temperature       = data.substring(0, delimiterIndex);
          pressure          = data.substring(delimiterIndex + 1);

        }
        ThingSpeak.setField(1, temperature);
        ThingSpeak.setField(2, pressure);
        int x = ThingSpeak.writeFields(chan_id, key);
        if(x == 200){
          display.drawString(3,0,"Channel update successful.");
        }
        else{
          display.drawString(3,0, "Problem updating channel.\n HTTP error code " + String(x));
        }

        Serial.print(data);
        //display.drawString(20,22, "température : " + temperature + " °C\npression: "+ pressure + " hPa");
        display.display();
      }
      else
      {
        display.drawString(20,22, "no data received");
        display.display();
      }
   
    }
    // print RSSI of packet
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
    display.drawString(20, 45, "RSSI:  ");
    display.drawString(70, 45, (String)LoRa.packetRssi());
    display.display();
    delay(20000);
  }
}