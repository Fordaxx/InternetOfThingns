#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define WIFI_SSID "*************"                    // enter your WIFI here
#define WIFI_PASSWORD "***************"
#define BOT_TOKEN "**************" // Telegram BOT Token (Get from BotFather)

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;             //Time via NTP
const int daylightOffset_sec = 3600;
unsigned long startTime = 0; // A variable for storing the start time


const unsigned long BOT_MTBS = 1000;  // mean time between scan messages
unsigned long last_time;
boolean flag = false;
int Dist;
boolean manualMode = 1;  // manual
boolean ledStatus;
int minDist = 50;
boolean enterStatus = 0;
unsigned long botLastTime;  // last time messages' scan has been done
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void pins() {
  pinMode(26, OUTPUT); // sets 26th and 33rd pins as outputs
  pinMode(33, OUTPUT);
}


void AutoLed(int Dist) {                          // it only works if manualMode = false (auto mode)
  if (!manualMode) {
    if (Dist <= minDist){
      ledON();
      startTime = millis();
      Serial.println(millis() - startTime);
      }
    if (millis() - startTime >= 5000) { // Check if 5 seconds have passed
        ledOFF(); 
        Serial.println(millis() - startTime);
    }
    }
 }


void ledON() {
  digitalWrite(26, HIGH);
  digitalWrite(33, LOW);       // turns on the led
  ledStatus = true;
}


void ledOFF() {
  digitalWrite(26, LOW);
  digitalWrite(33, LOW);      // turns off the led
  ledStatus = false;
}


void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  String answer;
  for (int i = 0; i < numNewMessages; i++) {
    telegramMessage& msg = bot.messages[i];
    Serial.println("Received " + msg.text);
    if (msg.text == "/help")
      answer = "So you need _help_, uh? me too! use /start or /status. Use /chmod to change your mode to manual or Ultrsasonic sensor mode";     // reaction to the message /help
    else if (msg.text == "/start")
      answer = "Welcome my new friend! You are the first *" + msg.from_name + "* I've ever met";  // reaction to the message /start. Using only once.
    else if (msg.text == "/status"){
      if (manualMode)
        answer = "Current mode is manual input";
      else
        answer = "Current mode is Ultrasonic sensor";
    }
    if (msg.text == "/chmod") {
      Serial.println("Current mode is" + manualMode);
      if (!manualMode)
        answer = "You switched your current mode to manual input mode";   
      else                                                                     // reaction to the message /chmod. Changing current mode to opposite
        answer = "You switched your current mode to Ultrasonic Sensor";
      manualMode = !manualMode;
      Serial.println("Current mode is" + manualMode);
    }
    if (manualMode) {                                                // it only works if manualMode = true. You can change manualMode by /chmod.
      pins();                                                         // if manualMode = true working manual input mode, else if manualMode = false working Ultrasonic sensor mode
      if (msg.text == "/ledon") {
        Serial.println("Current mode is" + manualMode);
        ledON();                                                      // reaction to the message /ledON
        answer = "led is on";
      }
      if (msg.text == "/ledoff") {
        ledOFF();                                                     // reaction to the message /ledOFF
        answer = "led is off";
      }
      else if(enterStatus){
        if (msg.text == "/setDist") {
        answer = "enter minimal distance";
        minDist = msg.text.toInt();
        }
      }   
    }
  bot.sendMessage(msg.chat_id, answer, "Markdown");
  }
}


void setup() {
  boolean flag = false;
  Serial.begin(115200);
  Serial.println();
  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org");  // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600) {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}


int getDistance() {
  if (millis() - last_time > 10000) {
    if (flag == true) {
      Dist = 70;
      flag = false;                        // Emulates Ultrasonic sensor
    } else if (flag == false) {
      Dist = 30;
      flag = true;
    }
    last_time = millis();
    Serial.println(Dist);
  }
  return Dist;
}


void loop() {
  pins();
  int dist = getDistance();
  AutoLed(dist);
  if (millis() - botLastTime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    botLastTime = millis();
  }
}
