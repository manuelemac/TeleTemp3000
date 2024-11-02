#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHT.h"

#define DHTPIN 4 

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Wifi network station credentials
#define WIFI_SSID "***"
#define WIFI_PASSWORD "***"

// Telegram BOT Token (Get from Botfather)
//chat url : @emr_ced_alarm_bot
//created with : @botfather
#define BOT_TOKEN "****"
#define CHAT_ID "****"


WiFiClientSecure secured_client;

UniversalTelegramBot bot(BOT_TOKEN, secured_client);

DHT dht(DHTPIN, DHTTYPE);

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

const float TEMP_ALERT_THRESHOLD = 28.0;
unsigned long bot_lastalert; // last time a temp alert was sent
const unsigned long BOT_MTAM = 300000; // min time between temp alerts


unsigned long bot_lasttime; // last time messages' scan has been done
float temperatureC;
float temperatureF;
float humidity;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Welcome to EMERGENCY CED Temperature control system"));

  dht.begin();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperatureC = dht.readTemperature();
  Serial.println(temperatureC);
  // Read temperature as Fahrenheit (isFahrenheit = true)
  temperatureF = dht.readTemperature(true);
  
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    if ((temperatureC > TEMP_ALERT_THRESHOLD) && (bot_lastalert == 0 || millis() - bot_lastalert > BOT_MTAM))
    {
      //bot.sendMessage(CHAT_ID, "cisti!!");
      String imgAndText  = "[";
      imgAndText += temperatureC;
      imgAndText += " C";
      imgAndText += "](https://j.gifs.com/qj5Dv2.gif)";
      bot.sendMessage(CHAT_ID, imgAndText, "Markdown");
      imgAndText = "Temperature: ";
      imgAndText += temperatureC;
      imgAndText += " C";
      bot.sendMessage(CHAT_ID, imgAndText);
      bot_lastalert = millis();
    }
    
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID )
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
    }
    else
    {
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;
      if (from_name == "")
        from_name = "Guest";
      if (text == "/temp")
      {   String msg = "Temperature is ";
          msg += temperatureC;
          msg += " C";
          bot.sendMessage(chat_id,msg, ""); 
      }
      if (text == "/tempF")
      {  
          String msg = "Temperature is ";
          msg += temperatureF;
          msg += " F";
          bot.sendMessage(chat_id,msg, ""); 
      }
      if (text == "/humidity")
      {  
          String msg = "Humidity is ";
          msg += humidity;
          msg += " %"; 
          bot.sendMessage(chat_id,msg, ""); 
      }
      if (text == "/start")
      {
        String welcome = "Welcome to EMERGENCY CED Temperature control system.\n\n";
        welcome += "Powered by : ESP32 + DHT11 sensor\n\n";
        welcome += "Threshold alert is: ";
        welcome += TEMP_ALERT_THRESHOLD;
        welcome += " C \n\n Commands";
        welcome += "\n /temp : Temperature in celcius \n";
        //welcome += "/tempF : Temperature in farenheit\n";
        welcome += "/humidity : Humidity\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
      }
    }
  }
}
