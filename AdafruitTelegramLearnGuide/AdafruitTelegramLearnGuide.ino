/*******************************************************************
 *  A project to demonstrate the Universal Arduino Telegram Library      
 *  on the Adafruit Feather Huzzah (ESP8266)                          
 *  
 *  Main Hardware:
 *  - Adafruit Feather Huzzah
 *  - Level Shifter
 *  - Neopixel Ring
 *  - Button
 *                                                                 
 *  Written by Brian Lough                                         
 *******************************************************************/

// ----------------------------
// Standard Libraries
// ----------------------------

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// ----------------------------
// Featured Libraries
// ----------------------------

#include <UniversalTelegramBot.h>
// For hosting Telegram Messenger bots on your ESP8266
// Available on the library manager (Universal Telegram)
// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <Adafruit_NeoPixel.h>
// For controlling the Addressable LEDs
// Available on the library manager (Adafruit Neopixel)
// https://github.com/adafruit/Adafruit_NeoPixel

#define RED_STATE 1
#define GREEN_STATE 2
#define BLUE_STATE 3
#define OFF_STATE 0

// ----------------------------
// Configurations - Update these (see learn guide for more details)
// ----------------------------

// Initialize Wifi connection to the router
char ssid[] = "Mikrotik";     // Your network SSID (name)
char password[] = "mypassword"; // Your network password

#define telegramBotToken "XXXXXX:YYYYYYYYYYYYYYYYYYYYYYYYYYYYYY"  // Your Bot Token
#define pushChatId "12345678" // The Telegram ID that you want to send a message to when the button is pressed

const int neopixelRingPin = 0; // data pin for the Neopixel Ring
const int buttonPin = 2; // pin for button

const int numberOfNeopixels = 16;

// ----------------------------
// End of area you need to change
// ----------------------------

Adafruit_NeoPixel neopixels = Adafruit_NeoPixel(numberOfNeopixels, neopixelRingPin, NEO_GRB + NEO_KHZ800);

WiFiClientSecure client;
UniversalTelegramBot bot(telegramBotToken, client);

unsigned long botDelay = 1000; // Delay between checking Telegram for new messages, in milliseconds
                               // This could be increased if you wanted to save energy or just check less often
                               // (Telegram has no limit to how often it can be checked)
                               
unsigned long botCheckDueTime; // variable for storing the next time Telegram is due to be checked

int neopixelRingState = OFF_STATE; // Store the current state of the Neopixel Ring

volatile bool buttonPressedFlag = false; // Used to set a flag that lets us know to handle the button in the loop
                                         // Needs to be set as volatile to be accesable when in a interupt

void setup() {

  neopixels.begin(); // This initializes the NeoPixel library.
  neopixels.setBrightness(128);
  
  Serial.begin(115200);
  
  connectToWifi();

  pinMode(buttonPin, INPUT_PULLUP);

  // NOTE:
  // It is important to use interupts when making network calls in your sketch
  // if you just checked the status of the button in the loop you might
  // miss the button press as the device might be busy checking Telegram
  // at the time of the button press
  attachInterrupt(buttonPin, buttonPressed, RISING);
}

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void buttonPressed() {
  Serial.println("button pressed");
  if(digitalRead(buttonPin) == HIGH)
  {
    buttonPressedFlag = true;
  }
  return;
}

void setNeopixelColour(uint32_t colour) {
  for(int i=0; i< numberOfNeopixels; i++) {
    neopixels.setPixelColor(i, colour);
  }
  neopixels.show();
}

String getNeopixelStateText(){
  switch(neopixelRingState){
    case RED_STATE:
      return "red";
    case GREEN_STATE:
      return "green";
    case BLUE_STATE:
      return "blue";
    case OFF_STATE:
      return "off";  
  }

  return "unknown";
}

void processNewTelegramMessages(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    // Chat ID is who we need to reply to
    String chatId = String(bot.messages[i].chat_id);
    
    // Text is the contents of the recieved message
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/red") {
      setNeopixelColour(neopixels.Color(255, 0, 0));
      neopixelRingState = RED_STATE;
      bot.sendMessage(chatId, "Neopixel ring is now red", "");
    }

    if (text == "/green") {
      setNeopixelColour(neopixels.Color(0, 255, 0));
      neopixelRingState = GREEN_STATE;
      bot.sendMessage(chatId, "Neopixel ring is now green", "");
    }

    if (text == "/blue") {
      setNeopixelColour(neopixels.Color(0, 0, 255));
      neopixelRingState = BLUE_STATE;
      bot.sendMessage(chatId, "Neopixel ring is now blue", "");
    }

    if (text == "/off") {
      setNeopixelColour(neopixels.Color(0, 0, 0));
      neopixelRingState = OFF_STATE;
      bot.sendMessage(chatId, "Neopixel ring is now off", "");
    }

    if (text == "/status") {
      String statusMessage = "Neopixel ring is currently: ";
      statusMessage.concat(getNeopixelStateText());
      bot.sendMessage(chatId, statusMessage, "");

    }

    if (text == "/options") {
      // Reply keyboard is a array of arrays.
      // The Array in the first position of the outer array makes up the buttons on the top row
      
      String keyboardJson = "[[\"/red\", \"/green\", \"/blue\"],[\"/status\"],[\"/off\"]]";
      bot.sendMessageWithReplyKeyboard(chatId, "Choose from one of the following options", "", keyboardJson, true);
    }

    if (text == "/start") {
      String welcome = "Hi " + from_name + "!\n";
      welcome.concat("Welcome to the Adafruit Learn Guide for the Universal Arduino Telegram Library.\n");
      welcome.concat("\n");
      welcome.concat("I know the following commands:\n");
      welcome.concat("/options : get a keyboard with all the options.\n");
      
      welcome.concat("/red : Turn the neopixel ring red.\n");
      welcome.concat("/green : Turn the neopixel ring green.\n");
      welcome.concat("/blue : Turn the neopixel ring blue.\n");
      welcome.concat("/off : Turn the neopixel ring blue.\n");
      welcome.concat("/status : Return the status of the neopixel ring.\n");
      
      bot.sendMessage(chatId, welcome, "Markdown");
    }
  }
}

void sendMessage() {
  String message = "Hello from your Feather Huzzah!";
  message.concat("\n");
  message.concat("I'm on a new line down here.");
  if (bot.sendMessage(pushChatId, message, "Markdown")) {
    Serial.println("TELEGRAM Successfully sent");
  }
  buttonPressedFlag = false;
}

void loop() {

  if (buttonPressedFlag) {
    sendMessage();
  }
  
  unsigned int now = millis();
  // Check if it the bot is due to check for new messages
  if (botCheckDueTime < now) {
    
    // getUpdates will return the number of new messages recieved
    // The parameter it takes is the ID of the last message recieved + 1
    // This is to let Telegram know that the last message has been processed   
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    // Note: Currently getUpdates will limit the amount of messages it will
    // bring back from Telegram due to the size of the JSON response.
    // We can get around this by repeating the check in a while loop as below
    
    while(numNewMessages) {
      Serial.println("got a response");
      processNewTelegramMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    botCheckDueTime = now + botDelay;
  }
}
