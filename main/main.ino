// Make it 1 to see all debug messages in Serial Monitor
#define DEBUG_SW 0
// Pins of Switches / Input
#define S1 12  // D6
#define S2 13  // D7
#define S3 14  // D5
#define S4 4   // D2
#define S5 5   // D1
// Pins of Relay (Appliances Control) / Output
#define R1 0   // D3
#define R2 2   // D4
#define R3 15  // D8
#define R4 3   // RX
#define R5 1   // TX

// Insert RTDB URLefine the RTDB URL */
#define FIREBASE_HOST ""
// Insert Firebase project API Key
#define API_KEY ""
#define FIREBASE_AUTH ""
// WiFi Credentials
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Function Declaration
void log(String message);
void processData();
void getDataFirebaseData(String url, int relayPin, bool *relayState);
void readDataFromFirebase();
void handleSwitchAction(bool switchState, int *prevState, bool *relayState, int relayPin, String url);

// switch state
int switchFlag1 = 0;
int switchFlag2 = 0;
int switchFlag3 = 0;
int switchFlag4 = 0;
int switchFlag5 = 0;
// relay state
bool relayState1 = LOW;
bool relayState2 = LOW;
bool relayState3 = LOW;
bool relayState4 = LOW;
bool relayState5 = LOW;
// Define Firebase Data object
FirebaseData fbdo;
FirebaseData firebaseData;
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;
int intValue = 0;
// Flag for syncing
bool local = false;
bool isSync = false;

void log(String message) {
  if (DEBUG_SW)
    Serial.println(message);
}

void setup() {
  Serial.begin(115200);
  // relay
  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);
  pinMode(R4, OUTPUT);
  pinMode(R5, OUTPUT);
  digitalWrite(R1, HIGH);
  digitalWrite(R2, HIGH);
  digitalWrite(R3, HIGH);
  digitalWrite(R4, HIGH);
  digitalWrite(R5, HIGH);
  // switch
  pinMode(S1, INPUT_PULLUP);
  pinMode(S2, INPUT_PULLUP);
  pinMode(S3, INPUT_PULLUP);
  pinMode(S4, INPUT_PULLUP);
  pinMode(S5, INPUT_PULLUP);
  // WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Wait for wifi for 10 secs
  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    if (millis() - ms > 10000)
      break;
  }
  // Set up FirebaseConfig object
  config.host = FIREBASE_HOST;
  config.api_key = API_KEY;
  // Set up FirebaseAuth object
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  // Initialize Firebase with the config and auth objects
  Firebase.begin(&config, &auth);
  delay(2000);
  log("Start Programming");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    log("Wifi Not Connected");
    local = true;
    isSync = false;
  } else {
    if (local) {
      // sync data from local to firebase
      Firebase.RTDB.setInt(&fbdo, "/kubo/light1/value", (relayState1) ? 1 : 0);
      Firebase.RTDB.setInt(&fbdo, "/kubo/light2/value", (relayState2) ? 1 : 0);
      Firebase.RTDB.setInt(&fbdo, "/kubo/light3/value", (relayState3) ? 1 : 0);
      Firebase.RTDB.setInt(&fbdo, "/kubo/light4/value", (relayState4) ? 1 : 0);
      Firebase.RTDB.setInt(&fbdo, "/kubo/outlet1/value", (relayState5) ? 1 : 0);
      local = false;
    }
    log("Wifi Connected");
    readDataFromFirebase();
  }
  processData();
}

void getDataFirebaseData(String url, int relayPin, bool *relayState) {
  if (Firebase.RTDB.getInt(&fbdo, url)) {
    if (fbdo.dataType() == "int") {
      intValue = fbdo.intData();
      log(intValue);
      (*relayState) = (intValue == 1) ? HIGH : LOW;
      digitalWrite(relayPin, (*relayState));
    } else {
      log(fbdo.errorReason());
    }
  }
}

void readDataFromFirebase() {
  if (Firebase.ready()) {
    getDataFirebaseData("/kubo/light1/value", R1, &relayState1);
    getDataFirebaseData("/kubo/light2/value", R2, &relayState2);
    getDataFirebaseData("/kubo/light3/value", R3, &relayState3);
    getDataFirebaseData("/kubo/light4/value", R4, &relayState4);
    getDataFirebaseData("/kubo/outlet1/value", R5, &relayState5);
  }
}

void handleSwitchAction(bool switchState, int *prevState, bool *relayState, int relayPin, String url) {
  if (switchState != (*prevState)) {
    // Check if the switch is pressed (state changed from HIGH to LOW)
    if (switchState == LOW) {
      // Toggle the relay state
      (*relayState) = !(*relayState);
      digitalWrite(relayPin, (*relayState));  // Update the relay state
      if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
        int convertedValue = (*relayState) ? 1 : 0;
        if (Firebase.RTDB.setInt(&fbdo, url, convertedValue)) {
          log("PASSED");
          log("PATH: " + fbdo.dataPath());
        } else {
          log("FAILED");
          log("REASON: " + fbdo.errorReason());
        }
      }
    }
  }
  (*prevState) = switchState;
}

void processData() {
  bool switchState1 = digitalRead(S1);
  bool switchState2 = digitalRead(S2);
  bool switchState3 = digitalRead(S3);
  bool switchState4 = digitalRead(S4);
  bool switchState5 = digitalRead(S5);

  handleSwitchAction(switchState1, &switchFlag1, &relayState1, R1, "/kubo/light1/value");
  handleSwitchAction(switchState2, &switchFlag2, &relayState2, R2, "/kubo/light2/value");
  handleSwitchAction(switchState3, &switchFlag3, &relayState3, R3, "/kubo/light3/value");
  handleSwitchAction(switchState4, &switchFlag4, &relayState4, R4, "/kubo/light3/value");
  handleSwitchAction(switchState5, &switchFlag5, &relayState5, R5, "/kubo/light3/value");
}