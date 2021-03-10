/*!
 * file OTACredentials.h
 *
 * This code is made for entering WiFi and Firebase Credentials
 * Over the Air using your web browser.
 * 
 *
 * Written by Sachin Soni for techiesms YouTube channel,
 * And edited by Deny Kurniawan to custom some preferences
 */

#include "Arduino.h"
#include "OTACredentials.h"

AsyncWebServer server(80); // Creating WebServer at port 80
WebSocketsServer webSocket = WebSocketsServer(81); // Creating WebSocket Server at port 81


// This is the webpage which is hosted on your ESP board, you can access this webpage by going to this address
// Address: 192.168.4.1

char _webpage[] PROGMEM = R"=====(

<html>
  <head>
    <script>
      var connection = new WebSocket('ws://'+location.hostname+':81/');
      connection.onopen = function () {
        connection.send('Connect ' + new Date()); 
      };
      connection.onerror = function (error) {
        console.log('WebSocket Error ', error);
      };
      connection.onmessage = function (e) {
        console.log('Server: ', e.data);
      };
        
      function credentials_rec() {
        var ssid = document.getElementById('ssid_cred').value;
        var pass = document.getElementById('pass_cred').value;
        var url  = document.getElementById('firebase_url').value;
        var key  = document.getElementById('firebase_key').value;
        var full_command = '#{"ssid":"'+ ssid +'", "pass":"' +pass +'","url":"'+ url +'","key":"'+ key +'"}';
        console.log(full_command);
        connection.send(full_command);
        location.replace('http://'+location.hostname+'/submit');
      }
    </script>
  </head>

  <body>
    <label for="ssid_cred">SSID Name:</label>
    <input type="text" id="ssid_cred"><br><br>
    <label for="pass_cred">Password:</label>
    <input type="text" id="pass_cred"><br><br>
    <lable for="firebase_url">Firebase Url:</label>
    <input type="text" id="firebase_url"><br><br>
    <lable for="firebase_key">Firebase Key:</label>
    <input type="text" id="firebase_key"><br><br>
    <button type="button" onclick=credentials_rec();>Submit</button>
  </body>
</html>
)=====";


void _webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] Get Text: %s\n", num, payload);

      if (payload[0] == '#') {
        String message = String((char*)( payload));
        message = message.substring(1);
        Serial.println(message);

        //JSON part
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, message);

        String ssid = doc["ssid"];
        String pass = doc["pass"];
        String url  = doc["url"];
        String key  = doc["key"];
        Serial.println(ssid); Serial.println(pass);


        // Clearing EEPROM
        if (ssid.length() > 0 && pass.length() > 0) {
          Serial.println("Clearing EEPROOM");
          for (int i = 0; i < 100; ++i) {
            EEPROM.write(i, 0);
          }
          // Storing in EEPROM
          Serial.println("Writing EEPROOM SSID:");
          for (int i = 0; i < ssid.length(); ++i) {
            EEPROM.write(i, ssid[i]);
            Serial.print("Wrote: ");
            Serial.println(ssid[i]);
          }
          Serial.println("Writing EEPROOM Password:");
          for (int i = 0; i < pass.length(); ++i) {
            EEPROM.write(32 + i, pass[i]);
            Serial.print("Wrote: ");
            Serial.println(pass[i]);
          }
          Serial.println("Writing EEPROOM Firebase url:");
          for (int i = 0; i < url.length(); ++i) {
            EEPROM.write(64 + i, url[i]);
            Serial.print("Wrote: ");
            Serial.println(url[i]);
          }
          Serial.println("writing eeprom firebase key:");
          for (int i = 0; i < key.length(); ++i)
          {
            EEPROM.write(64 + i, key[i]);
            Serial.print("Wrote: ");
            Serial.println(key[i]);
          }
          EEPROM.commit();
          delay(2000);

          //Restarting ESP board
          ESP.restart();
          break;
        }
     }
  }
}



void credentials::Erase_eeprom() {
  EEPROM.begin(512); //Initialasing EEPROM
  Serial.println("Erasing...");
    for (int i = 0; i < 100; ++i) {
      EEPROM.write(i, 0);
    }
   EEPROM.commit();
}

String credentials::EEPROM_Config() {
  EEPROM.begin(512); //Initialasing EEPROM
  Serial.println();
  Serial.println();

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM");


  for (int i = 0; i < 32; ++i)
  {
    ssid += char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(ssid);

  for (int i = 32; i < 64; ++i)
  {
    pass += char(EEPROM.read(i));
  }
  Serial.print("Password: ");
  Serial.println(pass);

  String firebase_url = "";
  for (int i = 64; i < 100; ++i)
  {
    firebase_url += char(EEPROM.read(i));
  }
  Serial.print("Firebase url: ");
  Serial.println(firebase_url);
  return firebase_url;
  
  String firebase_key = "";
  for (int i = 64; i < 100; ++i)
  {
    firebase_key += char(EEPROM.read(i));
  }
  Serial.print("Firebase Key: ");
  Serial.println(firebase_key);
  return firebase_key;

}



bool credentials::credentials_get()
{
  if (_testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return true;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    return false;
  }
}



void credentials::setupAP(char* softap_ssid, char* softap_pass)
{
  
  WiFi.disconnect();
  delay(100);
  WiFi.softAP(softap_ssid,softap_pass);
  _launchWeb();
  Serial.println("Server Started");
   webSocket.begin();
    webSocket.onEvent(_webSocketEvent);
}



bool credentials::_testWifi(){
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  char* my_ssid = &ssid[0];
  char* my_pass = &pass[0];

  WiFi.begin(my_ssid, my_pass);
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print(". ");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}



// This is the function which will be called when an invalid page is called from client
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}


void credentials::_createWebServer()
{
server.on("/", [](AsyncWebServerRequest * request) {
  request->send_P(200, "text/html", _webpage);
});

// Send a GET request to <IP>/get?message=<message>
server.on("/submit", HTTP_GET, [] (AsyncWebServerRequest * request) {
  String message;
    message = "Credentials received by ESP board!!!";
   request->send(200, "text/plain", message);
});

server.onNotFound(notFound);
server.begin();
  
}

void credentials::_launchWeb()
{
  Serial.println("");
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  _createWebServer();
  // Start the server

}

void credentials::server_loops()
{ 
     webSocket.loop();
}
