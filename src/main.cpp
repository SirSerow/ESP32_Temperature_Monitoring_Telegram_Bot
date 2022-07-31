#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <string>
#include <iostream>
#include <sstream>


using namespace std;

const char* ssid = "XXXXXXXXXXXXXX"; //Enter your Wi-Fi network SSID
const char* password = "XXXXXXX"; //Enter your Wi-Fi network password
string *data;

#define ONE_WIRE_BUS 15  //OneWire bus pin

#define BOT_TOKEN "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" //Telegram bot token
#define CHAT_ID "XXXXXXXXXXXX" //Telegram bot id

const unsigned long BOT_MTBS = 1000;
const int LEDpin = 23;
const char temp ='t';

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

  string get_temp() {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    stringstream stream;
    stream.precision(3);
    stream << fixed;
    stream << tempC;
    string data = stream.str();
    string fail;
    if  (tempC != DEVICE_DISCONNECTED_C) 
    {
      return data;
    }
    else
    {
      return fail;
    } 
  }

void handleNewMessages(int numNewMessages){
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following command get data from sensors.\n\n";
      welcome += "/t to measure temperature \n";
      bot.sendMessage(chat_id, welcome, "");
    }    

    if (text == "/t") {
      bot.sendMessage(chat_id, "Temperature is:", "");
      if (!get_temp().empty()) {
        bot.sendMessage(chat_id, get_temp().c_str(), "");
      }
      else {
        bot.sendMessage(chat_id, "Unable to recieve data from sensorr", "");
      }
    }
  }
}



void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  pinMode(LEDpin, OUTPUT);
  sensors.begin();
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
}