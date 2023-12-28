// Make it 1 to see all debug messages in Serial Monitor
#define DEBUG_SW 1
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

// Wifi Led
#define wifiLed D4
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
void with_internet();
void without_internet();
void getDataFirebaseData(String url, int relayPin);
void setDataFirebaseData(String url, int *switchFlag, int switchPin, int relayPin);
void readDataFromFirebase();
void log(String message);
void localSwitch(bool switchState, int *prevState, bool *relayState, int relayPin);

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
bool remote = false;

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
  // pinMode(R4, OUTPUT);
  // pinMode(R5, OUTPUT);
  digitalWrite(R1, HIGH);
  digitalWrite(R2, HIGH);
  digitalWrite(R3, HIGH);
  // digitalWrite(R4, HIGH);
  // digitalWrite(R5, HIGH);
  // switch
  pinMode(S1, INPUT_PULLUP);
  pinMode(S2, INPUT_PULLUP);
  pinMode(S3, INPUT_PULLUP);
  pinMode(S4, INPUT_PULLUP);
  pinMode(S5, INPUT_PULLUP);
  // led light
  pinMode(wifiLed, OUTPUT);
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
  // Firebase.begin(&config, &auth);
  Firebase.begin(&config, &auth);
  delay(2000);
  log("Start Programming");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    log("Wifi Not Connected");
    local = true;
    without_internet();
  } else {
    // sync data from local to firebase
    if (local == true) {
      // Firebase.RTDB.setInt(&fbdo, "/kubo/light1/value", digitalRead(R1));
      // Firebase.RTDB.setInt(&fbdo, "/kubo/light2/value", digitalRead(R2));
      // Firebase.RTDB.setInt(&fbdo, "/kubo/light3/value", digitalRead(R3));
      // Firebase.RTDB.setInt(&fbdo, "/kubo/light4/value", digitalRead(R4));
      // Firebase.RTDB.setInt(&fbdo, "/kubo/outlet1/value", digitalRead(R5));
      local = false;
    }
    log("Wifi Connected");
    readDataFromFirebase();
    with_internet();
  }
}

void getDataFirebaseData(String url, int relayPin) {
  if (Firebase.RTDB.getInt(&fbdo, url)) {
    if (fbdo.dataType() == "int") {
      intValue = fbdo.intData();
      log(intValue);
      if (intValue == 1) {
        digitalWrite(relayPin, LOW);
      } else {
        digitalWrite(relayPin, HIGH);
      }
    } else {
      log(fbdo.errorReason());
    }
  }
}

void setDataFirebaseData(String url, int *switchFlag, int switchPin, int relayPin) {
  if (digitalRead(switchPin) == LOW) {
    if ((*switchFlag) == 0) {
      log("Button Pressed");
      if (digitalRead(relayPin) == LOW) {
        digitalWrite(relayPin, HIGH);
        if (Firebase.RTDB.setInt(&fbdo, url, 0)) {
          log("PASSED");
          log("PATH: " + fbdo.dataPath());
        } else {
          log("FAILED");
          log("REASON: " + fbdo.errorReason());
        }
      } else {
        digitalWrite(relayPin, LOW);
        if (Firebase.RTDB.setInt(&fbdo, url, 1)) {
          log("PASSED");
          log("PATH: " + fbdo.dataPath());
        } else {
          log("FAILED");
          log("REASON: " + fbdo.errorReason());
        }
      }
      (*switchFlag) = 1;
    }
  }
  if (digitalRead(switchPin) == HIGH) {
    if ((*switchFlag) == 1) {
      // digitalWrite(relayPin, HIGH);
      // if (Firebase.RTDB.setInt(&fbdo, url, 0)) {
      //   log("PASSED");
      //   log("PATH: " + fbdo.dataPath());
      // } else {
      //   log("FAILED");
      //   log("REASON: " + fbdo.errorReason());
      // }
      (*switchFlag) = 0;
      // log("Switch OFF");
    }
  }
}

void readDataFromFirebase() {
  getDataFirebaseData("/kubo/light1/value", R1);
  getDataFirebaseData("/kubo/light2/value", R2);
  getDataFirebaseData("/kubo/light3/value", R3);
  // getDataFirebaseData("/kubo/light4/value", R4);
  // getDataFirebaseData("/kubo/outlet1/value", R5);
}

void with_internet() {
  setDataFirebaseData("/kubo/light1/value", &switchFlag1, S1, R1);
  setDataFirebaseData("/kubo/light2/value", &switchFlag2, S2, R2);
  setDataFirebaseData("/kubo/light3/value", &switchFlag3, S3, R3);
  // setDataFirebaseData("/kubo/light4/value", &switchFlag4, S4, R4);
  // setDataFirebaseData("/kubo/outlet1/value", &switchFlag5, S5, R5);
}

void localSwitch(bool switchState, int *prevState, bool *relayState, int relayPin) {
  if (switchState != (*prevState)) {
    // Check if the switch is pressed (state changed from HIGH to LOW)
    if (switchState == LOW) {
      // Toggle the relay state
      (*relayState) = !(*relayState);
      digitalWrite(relayPin, (*relayState));  // Update the relay state
    }
  }
  (*prevState) = switchState;
}

void without_internet() {
  bool switchState1 = digitalRead(S1);
  bool switchState2 = digitalRead(S2);
  bool switchState3 = digitalRead(S3);
  bool switchState4 = digitalRead(S4);
  bool switchState5 = digitalRead(S5);

  localSwitch(switchState1, &switchFlag1, &relayState1, R1);
  localSwitch(switchState2, &switchFlag2, &relayState2, R2);
  localSwitch(switchState3, &switchFlag3, &relayState3, R3);
  // localSwitch(switchState4, &switchFlag4, &relayState4, R4);
  // localSwitch(switchState5, &switchFlag5, &relayState5, R5);
}



// void Switch_Read()
// {
//   if (digitalRead(Switch1) == LOW)
//   {
//     Power = 0;
//     update_power_status();
//     load1 = !load1;
//     Relays();
//     if (Firebase.setString(firebaseData, "/L1", String(load1)))
//     {
//       Serial.println("PASSED");
//       Serial.println("PATH: " + firebaseData.dataPath());
//       Serial.println("TYPE: " + firebaseData.dataType());
//       Serial.println("ETag: " + firebaseData.ETag());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     else
//     {
//       Serial.println("FAILED");
//       Serial.println("REASON: " + firebaseData.errorReason());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     delay(300);
//   }
//   else if (digitalRead(Switch2) == LOW)
//   {
//     Power = 0;
//     update_power_status();
//     load2 = !load2;
//     Relays();
//     if (Firebase.setString(firebaseData, "/L2", String(load2)))
//     {
//       Serial.println("PASSED");
//       Serial.println("PATH: " + firebaseData.dataPath());
//       Serial.println("TYPE: " + firebaseData.dataType());
//       Serial.println("ETag: " + firebaseData.ETag());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     else
//     {
//       Serial.println("FAILED");
//       Serial.println("REASON: " + firebaseData.errorReason());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     delay(300);
//   }

//   else if (digitalRead(Switch3) == LOW)
//   {
//     Power = 0;
//     update_power_status();
//     load3 = !load3;
//     Relays();
//     if (Firebase.setString(firebaseData, "/L3", String(load3)))
//     {
//       Serial.println("PASSED");
//       Serial.println("PATH: " + firebaseData.dataPath());
//       Serial.println("TYPE: " + firebaseData.dataType());
//       Serial.println("ETag: " + firebaseData.ETag());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     else
//     {
//       Serial.println("FAILED");
//       Serial.println("REASON: " + firebaseData.errorReason());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     delay(300);
//   }

//   else if (digitalRead(Switch4) == LOW)
//   {
//     Power = 0;
//     update_power_status();
//     load4 = !load4;
//     Relays();
//     if (Firebase.setString(firebaseData, "/L4", String(load4)))
//     {
//       Serial.println("PASSED");
//       Serial.println("PATH: " + firebaseData.dataPath());
//       Serial.println("TYPE: " + firebaseData.dataType());
//       Serial.println("ETag: " + firebaseData.ETag());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     else
//     {
//       Serial.println("FAILED");
//       Serial.println("REASON: " + firebaseData.errorReason());
//       Serial.println("------------------------------------");
//       Serial.println();
//     }
//     delay(300);
//   }
// }