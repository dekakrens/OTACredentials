#include "OTACredentials.h"

credentials Credentials;

//Variables
char firebase_url[96];
char firebase_key[64];
bool connected_to_internet = false;
const int Erasing_button = 4;

//Provide credentials for your ESP server
char* esp_ssid = "ESP Over The Air";
char* esp_pass = "";

void setup(){

  Serial.begin(115200);
  pinMode(Erasing_button, INPUT);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.println(t);
    delay(1000);
  }

  // Press and hold the button 4seconds to erase all the credentials
  if (digitalRead(Erasing_button) == LOW){
    Credentials.Erase_eeprom();
  }

  String auth_string = Credentials.EEPROM_Config();
  auth_string.toCharArray(firebase_url, 33);
  auth_string.toCharArray(firebase_key, 33);

  if (Credentials.credentials_get()){
    //Configure firebase database
    //Blynk.config(auth_token);
    connected_to_internet = true;
  }
  else{
    Credentials.setupAP(esp_ssid, esp_pass);
    connected_to_internet = false;
  }

  if (connected_to_internet){
    //Write the setup part of your code here
  }
}

void loop(){
  Credentials.server_loops();

  if (connected_to_internet){
    //Write te loop part of your code here
  }
}
