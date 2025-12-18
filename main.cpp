/*
  Joke Machine - Final Version
  ESP32 + Make.com (Proxy & Logger) + Google Sheets
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Keypad.h>
#include <ESP32Servo.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- Make.com Configuration ---
String MAKE_JOKE_URL = "https://hook.eu1.make.com/zy7b0hejxuo8et7phv8spy09pi0jlm1v"; 
String MAKE_LOG_URL = "https://hook.eu1.make.com/oe62icuevcowayxvinrny6xah977xou3"; 

// --- Peripherals Setup ---
#define TFT_DC 2
#define TFT_CS 15
#define TFT_RST 4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const byte ROWS = 4; const byte COLS = 4;
char keys[ROWS][COLS] = { {'1','2','3','A'}, {'4','5','6','B'}, {'7','8','9','C'}, {'*','0','#','D'} };
byte rowPins[ROWS] = {27, 26, 25, 33}; byte colPins[COLS] = {32, 17, 16, 22};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int BUZZER_PIN = 14; 
#define BUZZER_CHANNEL 0 // 이거 추가함
enum MachineState { STATE_MENU, STATE_RATING };
MachineState currentState = STATE_MENU; 

String currentJoke = ""; 
String currentCategory = "";

void beep(int f, int d, int p) { tone(BUZZER_PIN, f); delay(d); noTone(BUZZER_PIN); delay(p); }

int normalizeScore(char scoreChar) {
  int s = scoreChar - '0';
  if (s < 1) s = 1;
  if (s > 5) s = 5;
  return s;
}

void buzzerLaugh(char scoreChar) {
  int score = normalizeScore(scoreChar);

  switch (score) {
    case 1:
      beep(700, 120, 200);
      break;

    case 2:
      beep(800, 120, 150);
      beep(800, 120, 150);
      break;

    case 3:
      for (int i = 0; i < 3; i++) beep(1000, 120, 100);
      break;

    case 4:
      beep(900, 120, 80);
      beep(1100, 120, 80);
      beep(1300, 120, 80);
      beep(1500, 150, 120);
      break;

    case 5:
      for (int i = 0; i < 6; i++) {
        beep(1600, 100, 60);
        beep(1800, 200, 120);
      }
      break;
  }
}

// --- Network Functions ---

String getJokeFromMake(String category) {
  String finalJoke = "Error: Network";
  
  if(WiFi.status() != WL_CONNECTED) { WiFi.reconnect(); delay(1000); return "Error: WiFi"; }

  WiFiClientSecure *client = new WiFiClientSecure;
  
  if(client) {
    client->setInsecure(); 
    client->setHandshakeTimeout(20000);

    HTTPClient http;
    String url = MAKE_JOKE_URL + "?category=" + category;
    
    Serial.printf("[GetJoke] Requesting '%s'...\n", category.c_str());

    if(http.begin(*client, url)) {
      int httpCode = http.GET();

      if (httpCode > 0) {
        finalJoke = http.getString();
        Serial.println("[GetJoke] Success!");
        
        if (finalJoke.length() < 2) finalJoke = "Error: Empty Data";
      } else {
        Serial.printf("[GetJoke] GET Error: %s\n", http.errorToString(httpCode).c_str());
        finalJoke = "Error: HTTP " + String(httpCode);
      }
      http.end();
    } else {
      Serial.println("[GetJoke] Connect Failed");
      finalJoke = "Error: Connect Failed";
    }
    delete client; 
  } else {
    Serial.println("[System] Heap Full");
    finalJoke = "Error: Heap Full";
  }
  
  return finalJoke;
}

bool sendLogToMake(String category, String joke, int rating) {
  bool success = false;

  if(WiFi.status() != WL_CONNECTED) { WiFi.reconnect(); delay(1000); return false; }

  Serial.println("[Logger] Sending Log...");
  
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client->setInsecure();
    client->setHandshakeTimeout(20000);

    HTTPClient http;
    if(http.begin(*client, MAKE_LOG_URL)) {
      http.addHeader("Content-Type", "application/json");

      StaticJsonDocument<1024> doc;
      doc["category"] = category;
      doc["joke"] = joke;
      doc["rating"] = rating; 

      String jsonPayload;
      serializeJson(doc, jsonPayload);

      int httpCode = http.POST(jsonPayload);
      
      if (httpCode > 0) {
        Serial.printf("[Logger] Success! Status: %d\n", httpCode);
        success = true;
      } else {
        Serial.printf("[Logger] Failed: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    }
    delete client; 
  } else {
    Serial.println("[Logger] Heap Full");
  }
  return success;
}

// --- Main Logic ---

void nextJoke(String category) {
  currentCategory = category;

  tft.setTextColor(ILI9341_WHITE);
  tft.println("\nFetching via Make...");

  String rawData = getJokeFromMake(category);

  while (rawData.startsWith("Error")) {
    tft.setTextColor(ILI9341_RED);
    tft.print("."); 
    Serial.println("[Retry] Fetching again...");
    delay(2000); 
    rawData = getJokeFromMake(category);
  }
  
  String englishPart = "";
  String koreanPart = "";
  
  int splitIndex = rawData.indexOf("|||"); 

  if (splitIndex > 0) {
    englishPart = rawData.substring(0, splitIndex);
    englishPart.trim(); 
    
    koreanPart = rawData.substring(splitIndex + 3);
    koreanPart.trim();
  } else {
    englishPart = rawData;
    koreanPart = "Translation missing.";
  }

  tft.fillScreen(ILI9341_BLACK); 
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_GREEN);
  tft.println(englishPart);

  Serial.println("\n[Korean Translation]");
  Serial.println(koreanPart);
  Serial.println("--------------------\n");
  
  currentJoke = englishPart; 
}

void showMenu() {
  currentState = STATE_MENU;
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println("\nSelect Category:\n1:Misc 2:Prog\n3:Dark 4:Pun\n5:Spooky 6:X-mas\n7:Any");
}

void showRatingThankYou(char score) {
  int ratingInt = score - '0';
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(2);
  tft.printf("\n\nRating: %d/5\nSaving Log...\n", ratingInt);
  buzzerLaugh(score);

  bool isSent = sendLogToMake(currentCategory, currentJoke, ratingInt);
  
  while (!isSent) {
    tft.setTextColor(ILI9341_RED);
    tft.print("Retry..."); 
    Serial.println("[Retry] Sending log again in 2s...");
    
    delay(2000); 
    isSent = sendLogToMake(currentCategory, currentJoke, ratingInt);
  }

  tft.setTextColor(ILI9341_GREEN);
  tft.println("Saved!");
  delay(1500); 
  showMenu(); 
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 6);

  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL); // 얘네 추가함

  tft.begin(); tft.setRotation(1);
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(2);
  tft.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(100); tft.print("."); }
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println("OK! IP=" + WiFi.localIP().toString());
  delay(1000); 
  showMenu();
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if (currentState == STATE_MENU) {
      String cat = "";
      switch (key) {
        case '1': cat = "Misc"; break; 
        case '2': cat = "Programming"; break;
        case '3': cat = "Dark"; break; 
        case '4': cat = "Pun"; break;
        case '5': cat = "Spooky"; break; 
        case '6': cat = "Christmas"; break;
        case '7': cat = "Any"; break;
      }
      if (cat.length() > 0) {
        tft.fillScreen(ILI9341_BLACK); tft.setCursor(0,0);
        tft.println("Selected: " + cat);
        nextJoke(cat); 
        currentState = STATE_RATING;
        tft.setTextColor(ILI9341_MAGENTA); tft.println("\nRate (1-5) or *");
      }
    } else if (currentState == STATE_RATING) {
      if (key >= '1' && key <= '5') showRatingThankYou(key); 
      else if (key == '*') showMenu();
    }
  }
  delay(10);
}
