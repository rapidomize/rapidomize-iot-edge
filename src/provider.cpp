#include <Arduino.h>
#include <Update.h>
// #include <ESPmDNS.h>

#include <cstring>

#include "provider.h"
#include "device.h"
#include "utils.h"
#include "tmpl.h"

namespace rpz{

#ifndef AP_SSID
#define AP_SSID "iot_edge"
#define AP_PWD ""
#endif


/* 
  Connect to the WiFi SSID - 'iot_edge'
  Go to http://192.168.4.1 in a web browser to configure the gateway
*/      
void ConProvider::homePage(AsyncWebServerRequest *request, int status, const char *err) {
  Serial.printf(PSTR("Home page - %d - %s\n"), status, err?err:"");
  
  char *dash = getDash();
  char *wifi = getWifi();
  char *mqtt = getMqtt();
  char *peri = getPeri();

  char *page = (char *) malloc(8192*4);
  if(wifi_ssid.length() == 0 || wifi_pwd.length() == 0){
    sprintf(page, main_tmpl, err?err:"", dash, wifi, "", "");
  }else{
    sprintf(page, main_tmpl, err?err:"", dash, wifi, mqtt, peri);
  }  

  request->send(status, "text/html", page);

  free(dash);
  free(wifi);
  free(mqtt);
  free(peri);
  free(page);
}

//page submitted
char *ConProvider::getDash(){
  char *fr = (char *) malloc(4096);
  sprintf(fr, dash_tmpl, "rpz-d2x2t2ux-we", RPZ_VERSION, ESP.getCpuFreqMHz(), ESP.getSketchSize()/1024, WiFi.localIP().toString().c_str(), "");
  return fr;
}

char * ConProvider::getWifi(){
  String ssidlst;
  char *fr = (char *) malloc(4096);
  for(int i=0;i<ssid_cnt && i < 20; i++){
    Serial.println(ssids[i].c_str());
    sprintf(fr, ssid_tmpl, ssids[i].c_str(), (wifi_ssid.equals(ssids[i]))?"checked":"", ssids[i].c_str());
    ssidlst += fr;
  }

  fr[0]='\0';
  sprintf(fr, wifi_tmpl, WiFi.localIP().toString().c_str(), ssidlst.c_str());
  return fr;
}

char * ConProvider::getMqtt(){
  char *fr = (char *) malloc(4096);
  sprintf(fr, mqtt_tmpl, host, port, clientId, username, "xxxxxxxx", topic, tls?"checked":"", ver, qos);
  return fr;
}

char *ConProvider::getPeri(){
  String peri;
  
  for(int i=0; i < MAX_PERIPHERALS && peripherals[i]; i++){
    char *fr = peripherals[i]->confpg();
    peri += fr;
    free(fr);
  }
  char *fr = (char *) malloc(4096*2);
  sprintf(fr, peri_tmpl, peri.c_str());
  return fr;
}

void ConProvider::onWifi(AsyncWebServerRequest *request){
  log("Setting up WiFi...");
  wifi_ssid.clear();
  wifi_pwd.clear();

  JsonDocument doc;
  toJson(request, doc);

  wifi_ssid = (const char *)doc["ssid"];
  wifi_pwd = (const char *)doc["pwd"];

  if(wifi_ssid.length() != 0 && wifi_pwd.length() != 0){
    if(connectWiFi(true)){
      request->send(200, "application/json", "{}");
      return;
    }     
  }
  request->send(400, "application/json", "{\"err\":\"Error connecting to WiFi! Check SSID/Password!\"}");
  //homePage(request, 400, "Error connecting to WiFi! Check SSID/Password!");
}

void ConProvider::onMqtt(AsyncWebServerRequest *request){
  log("Setting up MQTT...");

  JsonDocument doc;
  toJson(request, doc);

  host = (const char *)doc["host"];
  port = (int)doc["port"];
  clientId = (const char *)doc["clientId"];
  username = (const char *)doc["username"];
  password = (const char *)doc["password"];
  topic = (const char *)doc["topic"];
  tls = 1;//(char *)doc["tls").toInt();
  ver = (const char *)doc["ver"];
  qos = (int)doc["qos"];

  if(host.length() != 0 && port > 0 && clientId.length() != 0 
       && username.length() != 0 && password.length() != 0
       && topic.length() != 0 && tls >= 0
       && connectMQTT(true)){
      
      //update prefs
      prefs->putString("host", host);
      prefs->putShort("port", port);
      prefs->putString("clientId", clientId);
      prefs->putString("username", username);
      prefs->putString("password", password);
      prefs->putString("topic", topic);
      prefs->putShort("tls", tls);
      save();

      request->send(200, "application/json", "{}");
      return;
  } 
  request->send(400, "application/json", "{\"err\":\"Invalid MQTT Connection details!\"}");
}

void ConProvider::onPeri(AsyncWebServerRequest *request){
  log("Setting up Peripherals...");

  JsonDocument doc;
  toJson(request, doc);

  for(int i=0; i < MAX_PERIPHERALS && peripherals[i]; i++){
    peripherals[i]->init(&doc);
  }

  save();
  request->send(200, "application/json", "{}");
}

void ConProvider::restart(AsyncWebServerRequest *request){
    log("Restarting IoT Edge...");

    //commit the prefs & restart
    prefs->end();
    delay(500);
    ESP.restart();
}

void ConProvider::onReset(AsyncWebServerRequest *request){
  request->send(200, "application/json", "{\"err\":\"Resetting IoT Edge...!\"}");
  log("Resetting IoT Edge...");
  prefs->clear();
  restart(request);
}

void ConProvider::onUpgrade(AsyncWebServerRequest *request){
  restart(request);
}

//commit & restart
//TODO: mutex lock?
void ConProvider::save() {
  prefs->end();
  delay(100);
  prefs->begin("rpzc", false);
}

void ConProvider::scan() {
  
  bool hidden;
  Serial.println(F("Starting WiFi scan..."));

  ssid_cnt = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

  if (ssid_cnt == 0) {
    log("No WIFI networks found");
  } else if (ssid_cnt > 0) {
    log(PSTR("%d WIFI networks found:\n"), ssid_cnt);
  
    // Print unsorted scan results
    for (int8_t i = 0; i < ssid_cnt && i < 20; i++) {
      ssids[i] = WiFi.SSID(i);
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", ssids[i].c_str());
      Serial.print(" | ");
      Serial.printf("%4ld", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(10);

      yield();
    }
    Serial.println("-------------------------------------");
    WiFi.scanDelete();

  } else {
    log(PSTR("WiFi scan error %d\n"), ssid_cnt);
  }

  // Wait a bit before scanning again
  //delay(500);
}

String wifiStatus(){
    String status;
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL: status =  "WL_NO_SSID_AVAIL"; break;
      case WL_CONNECT_FAILED:  status = "WL_CONNECT_FAILED. Check SSID/Password!"; break;
      case WL_CONNECTION_LOST:  status = "WL_CONNECTION_LOST"; break;
      case WL_DISCONNECTED:  status = "WL_DISCONNECTED";  break;
      default:
        break;
    }
    return status;
}

bool ConProvider::connectWiFi(bool setup){
    if(WiFi.isConnected() && WiFi.SSID() == wifi_ssid) return false;
    
    log(PSTR("Connecting to WiFi ssid: %s, pwd: %s"), wifi_ssid.c_str(), wifi_pwd.c_str());

    WiFi.disconnect(true);  //disconnect from wifi AP to set wifi connection in STA
    //WiFi.mode(WIFI_AP_STA);    //init wifi mode
    WiFi.begin(wifi_ssid, wifi_pwd);
    int retry = 0;
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      log(PSTR("Cannot connect to WiFi, status: %s"), wifiStatus());

      if(retry++ > 10) {
        Utils::buzzer(2);
        retry = 0;
      }
      delay(500);
    }

    log(PSTR("Connected to WiFi SSID %s IP address: %s"), wifi_ssid.c_str(), WiFi.localIP().toString().c_str());

    /* if (MDNS.begin("rapidomize")) {
      Serial.println(F("MDNS responder started"));
    } */

    prefs->putString("wifi_ssid", wifi_ssid);
    prefs->putString("wifi_pwd", wifi_pwd);
    save();

    haswifi = true;
    return true;
}

//connect to MQTT server
bool ConProvider::connectMQTT(bool setup){
    if(host.length() == 0 || port == 0){
      log(PSTR("MQTT connection failed! Invalid Connection details: ssl://%s:%d"), host.c_str(), port);
      return false;
    } 

    if(!haswifi){
      connectWiFi();
    }
    // sslClient.setFingerprint(finger_print);
    //sslClient.setTrustAnchors(&cert);
    //sslClient.setInsecure();
    int retry = 0, cnt = 0;
    if(wifiClient.connected())  wifiClient.flush();
    else{
      wifiClient.stop();//FIXME: ??
      wifiClient.setTimeout(2);

      log(PSTR("Connecting to MQTT broker: ssl://%s:%d, with ClientID: %s Username: %s Password: xxxxxx topic: %s"), 
              host.c_str(), port, clientId.c_str(), username.c_str(), topic.c_str());//password.c_str()

      int err;
      while(!(err = wifiClient.connect(host.c_str(), port))){
        log(PSTR("Failed connecting to MQTT broker: ssl://%s:%d, ...retrying"), host.c_str(), port);
        if(WiFi.status() != WL_CONNECTED){
          log(PSTR("Cannot connect to WiFi, status: %s"), wifiStatus());
          Utils::buzzer(2);
          return false;
        }
        if(cnt++ > 25) {
          Utils::buzzer(1);
          cnt = 0;
        }
        delay(200);
      }
      mqttClient->setClient(wifiClient);
    }
    
    retry = 0, cnt = 0;
    while(!mqttClient->connected()
          && !mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str())){
      if(cnt++ > 3) {
        Utils::buzzer(2);
        log("Failed connecting to MQTT broker: ssl://%s:%d, with ClientID: %s Username: %s Password: xxxxxx...Check Credentials. Giving up after %d attempts", 
          host.c_str(), port, clientId.c_str(), username.c_str(), retry);
        return false;
      }
    }

    log(PSTR("Successfully connected to MQTT broker: ssl://%s:%d"), host.c_str(), port);
    mqttClient->setKeepAlive(90);
    // subscribe to a topic:
    //log(PSTR("Subscribing to topic: %s"), subtopic);
    //mqttClient->subscribe(subtopic);

    hasSetup = true;
    return true;
}

void ConProvider::toJson(AsyncWebServerRequest *request, JsonDocument &doc){
    String *data = (String *)request->_tempObject;

    if (!data) {
      request->send(400);
      return;
    }
    

    // no data ?
    if (!data->length()) {
      delete data;
      request->_tempObject = nullptr;
      request->send(400);
      return;
    }

    // deserialize and check for errors
    if (deserializeJson(doc, *data)) {
      delete data;
      request->_tempObject = nullptr;
      request->send(400);
      return;
    }

    //Serial.printf(PSTR("jdata: %s\n"), (*data).c_str());
    delete data;
    request->_tempObject = nullptr;
}

void ConProvider::init(PubSubClient *mqttClient, Peripheral **peripherals, Preferences *prefs){

  this->mqttClient = mqttClient;
  this->peripherals = peripherals;
  this->prefs = prefs;

  Serial.println(F("Init WiFi..."));
  WiFi.mode(WIFI_AP_STA);

  wifi_ssid = prefs->getString("wifi_ssid", "");
  wifi_pwd = prefs->getString("wifi_pwd", "");

  host = prefs->getString("host", "");
  port = prefs->getShort("port", 1883);
  clientId = prefs->getString("clientId", "cid");
  username = prefs->getString("username", "");
  password = prefs->getString("password", "");
  topic = prefs->getString("topic", "");
  tls = true;//prefs->getBool("tls");
  ver = prefs->getString("ver", "3.1.1");
  qos = prefs->getUInt("qos", 0); 

  scan();

  //SSE
  events.onConnect([this](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client connected! Last message ID is: %u\n", client->lastId());
    }
    //send event with message "Connected!", id current millis
    // and set reconnect delay to 1 second
    client->send("Connected", NULL, millis(),1000);
    //TODO: send the last 100 logs items
    this->initlog(); 
  });

  //aggregate all chunks
  auto aggregator = [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!index) {
        request->_tempObject = new String();
        ((String *)request->_tempObject)->reserve(total);
        // set timeout 30s
        request->client()->setRxTimeout(30);
      }
      if(!request->_tempObject) request->_tempObject = new String();
      ((String *)request->_tempObject)->concat((const char *)data, len);
  };

  server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->homePage(request);
  });
  server.on("/wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->onWifi(request);
  }, NULL, aggregator);
  server.on("/mqtt", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->onMqtt(request);
  }, NULL, aggregator);
  server.on("/peri", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->onPeri(request);
  }, NULL, aggregator);
  server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->onReset(request);
  }, NULL, aggregator);
  server.on("/fwurl", HTTP_POST, [this](AsyncWebServerRequest *request) {
    HTTPClient http;
    
    JsonDocument doc;
    toJson(request, doc);
    // String body;
    // serializeJson(doc,body);
    // Serial.printf("%s\n", body.c_str());
    /* const char * url = (const char *)doc["fw_url"];
    if(!strtok((char *)url, ".bin")){
      Serial.printf("URL Error: %s\n", url);
      request->send(400, "application/json", "{\"err\":\"Invalid firmware download url?\"}");
      return;
    } */
    const char * url = (const char *)doc["fw_url"];
    if(!url){
      Serial.println("URL Error: Null");
      request->send(400, "application/json", "{\"err\":\"Invalid firmware download url?\"}");
      return;
    }
    bool value = http.begin(this->wifiClient, url);
    Serial.printf("%d", value);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        // Get the stream pointer to the firmware data
        WiFiClient* stream = http.getStreamPtr();

        // Start the update process
        if (!Update.begin(http.getSize(), U_FLASH)) {
          Update.printError(Serial); //Update.errorString()
          request->send(400, "application/json", "{\"err\":\"Firmware update begin failed...!\"}");
          http.end();
          return;
        }

        // Write the stream to the flash memory
        Update.writeStream(*stream);
        if (!Update.end()) {
          Update.printError(Serial);
          request->send(400, "application/json", "{\"err\":\"Firmware update ending failed...!\"}");
          http.end();
          return;
        }
        /* if (Update.canRollBack()) { // Optional: check if rollback is possible
            Serial.println("Can roll back to previous version.");
        } */
        log("Firmware update successfully! Device will be rebooted shortly to start new firmware");
        request->send(200, "application/json", "{\"err\":\"Firmware update successfully! Device will be rebooted shortly to start new firmware!\"}");
        delay(500);
        this->onUpgrade(request);
    } else {
        Serial.printf("HTTP Update failed. Error: %d\n", httpCode);
        request->send(400, "application/json", "{\"err\":\"Firmware update failed...!\"}");
    }
    http.end();
  }, NULL, aggregator);
  server.on("/fwfile", HTTP_POST, [this](AsyncWebServerRequest *request) {
      if (request->getResponse()) {
        Serial.println("response already created");
        return;
      }
      
      if(fwupdated) {
        fwupdated = false;
        log("Firmware image uploaded successfully! Device will be rebooted shortly to start new firmware");
        request->send(200, "application/json", "{\"err\":\"Firmware image uploaded successfully! Device will be rebooted shortly to start new firmware!\"}");
        delay(500);
        this->onUpgrade(request);
        return;
      }
      request->send(400, "application/json", "{\"err\":\"Firmware image upload failed...!\"}");
  },
  [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      Serial.printf("Upload[%s]: index=%u, len=%u, final=%d\n", filename.c_str(), index, len, final);

      if (request->getResponse()) {
        Serial.println("Firmware upload aborted");
        return;
      }

      // start a new content-disposition upload
      if (!index) {
        // list all parameters
        const size_t params = request->params();
        for (size_t i = 0; i < params; i++) {
          const AsyncWebParameter *p = request->getParam(i);
          Serial.printf("Param[%u]: %s=%s, isPost=%d, isFile=%d, size=%u\n", i, p->name().c_str(), p->value().c_str(), p->isPost(), p->isFile(), p->size());
        }

        // get the content-disposition parameter
        const AsyncWebParameter *p = request->getParam(asyncsrv::T_name, true, true);
        if (p == nullptr) {
          request->send(400, "application/json", "{\"err\":\"Missing firmware binary?\"}");
          return;
        }

        // determine upload type based on the parameter name
        if (p->value() == "fw_file") {
          Serial.printf("Firmware image upload for file: %s\n", filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
            Update.printError(Serial);
            request->send(400, "application/json", "{\"err\":\"Firmware update begin failed...!\"}");
            return;
          }
        } else {
          Serial.printf("Unknown upload type for file: %s\n", filename.c_str());
          request->send(400, "application/json", "{\"err\":\"Unknown Firmware upload type...!\"}");
          return;
        }
      }

      // some bytes to write ?
      if (len) {
        if (Update.write(data, len) != len) {
          Update.printError(Serial);
          Update.end();
          request->send(400, "application/json", "{\"err\":\"Firmware update write failed...!\"}");
          return;
        }
      }

      // finish the content-disposition upload
      if (final) {
        if (!Update.end(true)) {
          Update.printError(Serial);
          request->send(400, "application/json", "{\"err\":\"Firmware update ending failed...!\"}");
          return;
        }
        fwupdated = true;
        // success response is created in the final request handler when all uploads are completed
        Serial.printf("Firmware written successfully - file %s\n", filename.c_str());
      }
  });

  //provide page for config or reconfig
  if(wifi_ssid.length() == 0 || wifi_pwd.length() == 0){
    WiFi.disconnect(); //if any previous connections?
    delay(100);
    wifi_ssid.clear();
    wifi_pwd.clear();

    Serial.printf(PSTR("Creating AP %s"), AP_SSID);
    WiFi.softAP(AP_SSID, NULL);//AP_PWD
    
    IPAddress myIP = WiFi.softAPIP();
    Serial.printf(PSTR(" IP address: %s\n"), myIP.toString().c_str());
  }else{
    connectWiFi();
  }

  server.begin();
  
  connectMQTT(true);
}

}