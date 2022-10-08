// including the required libraries

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiManager.h>

// defining the token and ID for the telegram bot

#define BOTtoken "5507140140:AAHZppNSL7nYTuXxcwIDJBKOL2wPRx170Hc"
#define CHAT_ID "1826295642"

// defining the Input output ports

#define Sensor D1
#define Is_Wifi D2
#define DOOR_SENSOR_PIN D3
#define Buzzer D0
// defining the inner variables

unsigned int currentDoorState; // current state of door sensor
unsigned int lastDoorState;    // previous state of door sensor
unsigned int tot;
unsigned int prev;
unsigned int now;
bool value;
//String text = bot.messages[i].text;
WiFiManager wm;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

//const int ledPin = 16;

void handleNewMessages(int numNewMessages) {
//  Serial.println("handleNewMessages");
//  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String text = bot.messages[i].text;
//    String from_name = bot.messages[i].from_name;
//    if (from_name == "") from_name = "Guest";

    if (text == "/off") {
      digitalWrite(Buzzer, LOW);    // turn the LED off (LOW is the voltage level)
      bot.sendSimpleMessage("", "Buzzer is turned off", "");
    }
    else if (text == "/start") {
      String welcome = "Hi this is the home sequrity system if the buzzer is runnign wirte /off to turn it off";
      bot.sendSimpleMessage("", welcome, "");
    }
  }
}

void setup()
{
    tot = 1;
    pinMode(Sensor, INPUT);
    //    pinMode(LEDR, OUTPUT);
    pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); // set arduino pin to input pull-up mode
    pinMode(Is_Wifi, OUTPUT);
    Serial.begin(115200);
    configTime(0, 0, "pool.ntp.org");                // get UTC time via NTP
    client.setTrustAnchors(&cert);                   // Add root certificate for api.telegram.org
    currentDoorState = digitalRead(DOOR_SENSOR_PIN); // read state
    WiFi.mode(WIFI_STA);
    if (wm.autoConnect("HomeSequritySestem"))
    {
        bot.sendMessage(CHAT_ID, "Home Sequrity System has started", "");
    }
    prev = millis() / 1000;
}
void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        wm.resetSettings();
        digitalWrite(Is_Wifi, LOW);
        ESP.restart();
    }
    else
    {
        digitalWrite(Is_Wifi, HIGH);
        value = digitalRead(Sensor);
        Serial.println(value);
        delay(500);
        now = millis() / 1000;
        if (now - prev <= 30 && tot >= 6)
        {
            Serial.println("Motion Detected");
            //        digitalWrite(LEDR, HIGH);
            bot.sendMessage(CHAT_ID, "Motion detected!!", "");
            tot = 1;
        }
        if (now - prev >= 30)
        {
            prev = now;
            tot = 1;
        }

        if (value == 1)
        {
            tot += 1;
        }
        else if (value == 0)
        {
            //        digitalWrite(LEDR, LOW);
        }

        lastDoorState = currentDoorState;                // save the last state
        currentDoorState = digitalRead(DOOR_SENSOR_PIN); // read new state

        if (lastDoorState == LOW && currentDoorState == HIGH)
        {   // state change: LOW -> HIGH
            //        Serial.println("The door-opening event is detected");
            digitalWrite(Buzzer, LOW);
            bot.sendMessage(CHAT_ID, "The door is opened", "");
            
        }
        else if (lastDoorState == HIGH && currentDoorState == LOW)
        { // state change: HIGH -> LOW
            bot.sendMessage(CHAT_ID, "The door is closed", "");
            //        Serial.println("The door-closing event is detected");
            // TODO: turn off alarm, light or send notification ...
        }
    }
    if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
}
