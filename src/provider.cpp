#include <Arduino.h>
#include <Update.h>
#include <NetworkClientSecure.h>
// #include <ESPmDNS.h>
// #include <LittleFS.h>

//ETH_PHY_LAN8720 #definition overrides for ETH.h
#define ETH_PHY_TYPE        ETH_PHY_LAN8720
#define ETH_PHY_ADDR         0      //I2C-address of Ethernet PHY 0 or 1
#define ETH_PHY_MDC         23
#define ETH_PHY_MDIO        18
#define ETH_PHY_POWER       -1
#define ETH_CLK_MODE        ETH_CLOCK_GPIO17_OUT

//above #definitions will be used by the following header
#include <ETH.h>

#include <cstring>

#include "provider.h"
#include "device.h"
#include "utils.h"
#include "tmpl.h"
// #include "sha1.h"
#include "sha256.h"
#include "cert.h"


namespace rpz{

const String AP_SSID="iot_edge";
const char * RPZC_HOST="iot-edge.local";
const String RPZ_PL = "ics.rapidomize.com";

const char *USR="admin";
const char *PWD="changeit";

const char *AP="ap";
const char *WIFI="wifi";

static const int PG_SIZE  = 1024 * 32; //is this too large?
static const int TAB_SIZE = 1024 * 4;

// SHA1 hasher;
static SHA256 hasher;

static bool authst = false;
static hw_timer_t *timer;

//TODO: Semaphore/mux
//https://docs.espressif.com/projects/arduino-esp32/en/latest/api/timer.html
void IRAM_ATTR onTicks() {
  authst = false;
}

void disableTimer(bool rm){
  if(timer){
    if(!rm){
      timerStop(timer);
      return;
    }
    timerDetachInterrupt(timer);
    timerEnd(timer);
  }
  timer = nullptr;
}

void setTimer(){
  if(timer){
    timerRestart(timer);
    return;
  }
  //1 MHz (1 tick = 1 microsecond)
  timer = timerBegin(1000000L); 
  timerAttachInterrupt(timer, &onTicks);
  
  //trigger every 30 min; 1 second = 1,000,000 ticks
  timerAlarm(timer, 30 * 60 * 1000000L, true, 0);
}

/* 
  Connect to the WiFi SSID - 'iot_edge'
  Go to http://192.168.4.1 in a web browser to configure the gateway
*/      
void ConProvider::homePage(AsyncWebServerRequest *request, int status, const char *err) {
  Serial.printf("Page Loading - %d - %s\n", status, err?err:"");
  //yield();

  char *page = (char *) malloc(PG_SIZE);
  if(!page){
    perror("Unable to allocate memory!");
    request->send(400, "application/json", "{\"err\":\"Unable to allocate memory!\"}");
    return;
  }

  if(!authst){
    Serial.printf("Auth Page Loading - %d - %s\n", status, err?err:"");
    sprintf(page, main_tmpl, err?err:"", "none", auth_tmpl, "", "", "", ""); 
    request->send(status, "text/html", page);
  }else{
    char *dash = getDash();
    char *wifi = getWifi();
    char *mqtt = getMqtt();
    char *peri = getPeri();

    char *fr = (char *) malloc(TAB_SIZE);
    sprintf(fr, tabs_tmpl, RPZ_VERSION, prefs->getBool(AP, true)?"checked":"", 
        prefs->getBool(WIFI, true)?"checked":"", !ethSt?"disabled":"");
    
    if(wifi_ssid.length() == 0 || wifi_pwd.length() == 0){
      sprintf(page, main_tmpl, err?err:"", "block", dash, wifi, "", "", fr);
    }else{
      sprintf(page, main_tmpl, err?err:"", "block", dash, wifi, mqtt, peri, fr);
    }  

    request->send(status, "text/html", page);

    free(fr);
    free(dash);
    free(wifi);
    free(mqtt);
    free(peri);
  }
  free(page);
}

//page submitted
char *ConProvider::getDash(){
  char *fr = (char *) malloc(TAB_SIZE);
  sprintf(fr, dash_tmpl, DEVICE_MODEL, RPZ_VERSION, ESP.getCpuFreqMHz(), ESP.getSketchSize()/1024, 
      WiFi.localIP().toString().c_str(),  ETH.localIP().toString().c_str(), "");
  return fr;
}

char * ConProvider::getWifi(){
  String ssidlst;
  char *fr = (char *) malloc(TAB_SIZE);
  for(int i=0;i<ssid_cnt && i < 20; i++){
    Serial.println(ssids[i].c_str());
    sprintf(fr, "<div><input type=\"radio\" name=\"ssid\" value=\"%s\" %s> <label>%s</label></div>", 
              ssids[i].c_str(), (wifi_ssid.equals(ssids[i]))?"checked":"", ssids[i].c_str());
    ssidlst += fr;
  }

  fr[0]='\0';
  sprintf(fr, netwrk_tmpl, WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), ssidlst.c_str(),  
      ETH.localIP().toString().c_str(), ETH.macAddress().c_str());
  return fr;
}

char * ConProvider::getMqtt(){
  char *fr = (char *) malloc(TAB_SIZE);
  sprintf(fr, mqtt_tmpl, host.c_str(), port, tls?"checked":"", clientId.c_str(), username.c_str(), "xxxxxxxx", 
    topic.c_str(), ver.c_str(), qos, rpzfmt?"checked":"");
  return fr;
}

char *ConProvider::getPeri(){
  String peri;
  
  for(int i=0; i < MAX_PERIPHERALS && peripherals[i]; i++){
    char *fr = peripherals[i]->confpg();
    peri += fr;
    free(fr);
  }
  //FIXME: use String instead
  char *fr = (char *) malloc(TAB_SIZE*2);
  sprintf(fr, peri_tmpl, peri.c_str());
  return fr;
}

/*char *ConProvider::getTmpl(String& name){
   File file = LittleFS.open(name, "r");
  if (file) {
      size_t size = file.size();
      char* buf = (char*)malloc(size + 1);
      
      if (buf) {
          file.readBytes(buf, size);
          buf[size] = '\0';
      }else buf[0] = '\0';
      file.close();
      return buf;
  }else{
    log("Invalid template file %s.html", name.c_str());
  }
  return nullptr; 
}*/

bool ConProvider::checkAuth(AsyncWebServerRequest *request){
  if(!authst){
    request->send(200, "application/json", "{\"urlp\":\"/\"}");
    return false;
  }
  return true;
}
void ConProvider::onAuth(AsyncWebServerRequest *request){
  yield();
  
  JsonDocument doc;
  toJson(request, doc);

  String usr = (const char *)doc["usr"];
  String pwd = (const char *)doc["pwd"];
  
  if(usr && pwd){
    pwd = hasher.sha256(pwd.c_str());

    String usrp = prefs->getString("usr", USR);
    String pwdp = prefs->getString("pwd", hasher.sha256(PWD));

    log("login...%s %s %s %s", usr, usrp, pwd, pwdp);
    if(usr == usrp && pwd == pwdp){//usr.length() != 0 && pwd.length() != 0 &&
      authst = true;
      request->send(200, "application/json", "{\"urlp\":\"/\"}");

      setTimer();
      return;
    }
  }
  request->send(400, "application/json", "{\"err\":\"Invalid Credentials!\"}");
}

void ConProvider::onWifi(AsyncWebServerRequest *request){
  yield();
  log("Setting up WiFi...");
  wifi_ssid.clear();
  wifi_pwd.clear();

  JsonDocument doc;
  toJson(request, doc);

  wifi_ssid = (const char *)doc["ssid"];
  wifi_pwd = (const char *)doc["pwd"];

  if(wifi_ssid.length() != 0 && wifi_pwd.length() != 0){
    if(connectWiFi(true)){
      char msg[128];
      sprintf(msg, "{\"ip\":\"%s\"}", WiFi.localIP().toString().c_str());
      request->send(200, "application/json", msg);
      
      return;
    }     
  }
  request->send(400, "application/json", "{\"err\":\"Error connecting to WiFi! Check SSID/Password!\"}");
}

void ConProvider::onMqtt(AsyncWebServerRequest *request){
  yield();
  log("Setting up MQTT...");

  JsonDocument doc;
  toJson(request, doc);

  host = (const char *)doc["host"];
  port = (short)doc["port"];
  clientId = (const char *)doc["clientId"];
  username = (const char *)doc["username"];
  password = (const char *)doc["password"];
  topic = (const char *)doc["topic"];
  wastls = tls;
  tls = doc["tls"]? strcmp((const char*)doc["tls"], "on") == 0 :false;
  ver = (const char *)doc["ver"];
  qos = (uint8_t)doc["qos"];
  rpzfmt = doc["rpzfmt"]? strcmp((const char*)doc["rpzfmt"], "on") == 0 :false;

  if(connectMQTT(true)){
      //update prefs
      prefs->putString("host", host);
      prefs->putShort("port", port);
      prefs->putString("clientId", clientId);
      prefs->putString("username", username);
      prefs->putString("password", password);
      prefs->putString("topic", topic);
      prefs->putBool("tls", tls);
      prefs->putString("ver", ver);
      prefs->putShort("qos", qos);
      prefs->putBool("rpzfmt", rpzfmt);
      save();

      request->send(200, "application/json", "{}");
      return;
  } 
  request->send(400, "application/json", "{\"err\":\"Invalid MQTT Connection details!\"}");
}

void ConProvider::onPeri(AsyncWebServerRequest *request){
  yield();
  log("Setting up Peripherals...");

  JsonDocument doc;
  toJson(request, doc);

  for(int i=0; i < MAX_PERIPHERALS && peripherals[i]; i++){
    peripherals[i]->init(&doc);
  }

  save();
  request->send(200, "application/json", "{}");
}

void ConProvider::onPrefs(AsyncWebServerRequest *request){
  yield();
  JsonDocument doc;
  toJson(request, doc);

  String pwd = (const char *)doc["pwd"];
  String cpwd = (const char *)doc["cpwd"];
  String ap = (const char *)doc[AP];
  String wifip = (const char *)doc[WIFI];

  if(pwd.length() != 0){
    /* if(pwd.length() < 8){
      request->send(400, "application/json", "{\"err\":\"Password is too small?\"}");
      return;
    } */
    if(pwd != PWD && pwd == cpwd){
      prefs->putString("pwd", hasher.sha256(pwd.c_str()));
    } else {
      request->send(400, "application/json", "{\"err\":\"Password does not match?\"}");
      return;
    }
  }

  bool bap = ap && ap == "on";
  prefs->putBool(AP, bap);
  if(!bap){
    WiFi.mode(WIFI_STA);   
  }else{
    WiFi.mode(WIFI_AP_STA);   
  }

  bool bwifi = wifip && wifip == "on";
  if(!bwifi){
    if(ethSt && ETH.connected()){
      //log("Setting WIFI_OFF");
      //WiFi.disconnect();
      prefs->putBool(WIFI, bwifi);
      WiFi.mode(WIFI_OFF);   
    }else{
      request->send(400, "application/json", "{\"err\":\"Unable to establish Ethernet connection, so WiFi is not disabled!\"}");
      return;
    }
  }else{
    prefs->putBool(WIFI, bwifi);
    connectWiFi(); 
  } 

  request->send(200, "application/json", "{}");
  save();
}

void ConProvider::restart(AsyncWebServerRequest *request){
  yield();
  log("Restarting IoT Edge...");

  //commit the prefs & restart
  prefs->end();
  delay(500);
  ESP.restart();
}

void ConProvider::onReset(AsyncWebServerRequest *request){
  yield();
  request->send(200, "application/json", "{\"err\":\"Resetting IoT Edge...!\"}");
  log("Resetting IoT Edge...");
  prefs->clear();
  restart(request);
}

void ConProvider::onUpgrade(AsyncWebServerRequest *request){
  restart(request);
}

void ConProvider::onFwUrl(AsyncWebServerRequest *request){
  JsonDocument doc;
  toJson(request, doc);

  const char * url = (const char *)doc["fw_url"];
  if(!url){
    Serial.println("URL Error: Null");
    request->send(400, "application/json", "{\"err\":\"Invalid firmware download url?\"}");
    return;
  }

  yield();

  HTTPClient http;
  NetworkClientSecure net;
  net.setCACert(GH_CERT);
  if (!net.connect("github.com", 443, 1000)) {
    request->send(400, "application/json", "{\"err\":\"Firmware update connection failed...!\"}");
    return;
  }
  delay(100);
  bool value = http.begin(net, url);
  Serial.printf("%d", value);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
      // Get the stream pointer to the firmware data
      NetworkClient* stream = http.getStreamPtr();

      // Start the update process
      if (!Update.begin(http.getSize(), U_FLASH)) {
        log("Firmware update begin failed: %s", Update.errorString());
        //Update.printError(Serial);
        request->send(400, "application/json", "{\"err\":\"Firmware update begin failed...!\"}");
        http.end();
        return;
      }

      // Write the stream to the flash memory
      Update.writeStream(*stream);
      if (!Update.end()) {
        log("Firmware update ending failed: %s", Update.errorString());
        //Update.printError(Serial);
        request->send(400, "application/json", "{\"err\":\"Firmware update ending failed...!\"}");
        http.end();
        return;
      }
      /* if (Update.canRollBack()) { // Optional: check if rollback is possible
          Serial.println("Can roll back to previous version.");
      } */
      const char *msg = "Firmware update successfully! Device will be rebooted shortly to start new firmware";
      log(msg);
      request->send(200, "application/json", 
          "{\"err\":\"Firmware update successfully! Device will be rebooted shortly to start new firmware!\"}");
      //delay(500);
      this->onUpgrade(request);
  } else {
      Serial.printf("HTTP Update failed. Error: %d\n", httpCode);
      request->send(400, "application/json", "{\"err\":\"Firmware update failed...!\"}");
  }
  http.end();
}

void ConProvider::onFwFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  Serial.printf("Upload[%s]: index=%u, len=%u, final=%d\n", filename.c_str(), index, len, final);

    if (request->getResponse()) {
      log("Firmware upload aborted");
      return;
    }

    // start a new content-disposition upload
    if (!index) {
      // list all parameters
      const size_t params = request->params();
      for (size_t i = 0; i < params; i++) {
        const AsyncWebParameter *p = request->getParam(i);
        Serial.printf("Param[%u]: %s=%s, isPost=%d, isFile=%d, size=%u\n", i, p->name().c_str(), 
            p->value().c_str(), p->isPost(), p->isFile(), p->size());
      }

      // get the content-disposition parameter
      const AsyncWebParameter *p = request->getParam(asyncsrv::T_name, true, true);
      if (p == nullptr) {
        log("Missing firmware binary");
        request->send(400, "application/json", "{\"err\":\"Missing firmware binary?\"}");
        return;
      }

      // determine upload type based on the parameter name
      if (p->value() == "fw_file") {
        log("Firmware image upload file: %s\n", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
          log("Firmware update begin failed: %s", Update.errorString());
          //Update.printError(Serial);
          request->send(400, "application/json", "{\"err\":\"Firmware update begin failed...!\"}");
          return;
        }
      } else {
        log("Unknown upload file type: %s\n", filename.c_str());
        request->send(400, "application/json", "{\"err\":\"Unknown Firmware upload type...!\"}");
        return;
      }
    }

    // some bytes to write ?
    if (len) {
      if (Update.write(data, len) != len) {
        log("Firmware update write failed: %s", Update.errorString());
        //Update.printError(Serial);
        Update.end();
        request->send(400, "application/json", "{\"err\":\"Firmware update write failed...!\"}");
        return;
      }
    }

    // finish the content-disposition upload
    if (final) {
      if (!Update.end(true)) {
        log("Firmware update ending failed: %s", Update.errorString());
        //Update.printError(Serial);
        request->send(400, "application/json", "{\"err\":\"Firmware update ending failed...!\"}");
        return;
      }
      fwupdated = true;
      // success response is created in the final request handler when all uploads are completed
      log("Firmware written successfully - file %s\n", filename.c_str());
    }
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

    // Serial.printf("jdata: %s\n", (*data).c_str());
    delete data;
    request->_tempObject = nullptr;
}

//commit & restart
//TODO: mutex lock?
void ConProvider::save() {
  prefs->end();
  delay(100);
  prefs->begin("rpzc", false);
}

void ConProvider::scan() {
  yield();
  bool hidden;
  Serial.println("Starting WiFi scan...");

  ssid_cnt = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

  if (ssid_cnt == 0) {
    log("No WIFI networks found");
  } else if (ssid_cnt > 0) {
    log("%d WIFI networks found:\n", ssid_cnt);
  
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
    log("WiFi scan error %d\n", ssid_cnt);
  }
}

const char * ConProvider::wifiStatus(){
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL: wifiStStr =  "WL_NO_SSID_AVAIL"; break;
      case WL_CONNECT_FAILED:  wifiStStr = "WL_CONNECT_FAILED. Check SSID/Password!"; break;
      case WL_CONNECTION_LOST:  wifiStStr = "WL_CONNECTION_LOST"; break;
      case WL_DISCONNECTED:  wifiStStr = "WL_DISCONNECTED";  break;
      default:
        break;
    }
    return wifiStStr.c_str();
}

bool ConProvider::connectETH(bool wsetup){
  return ethSt;
}

bool ConProvider::connectWiFi(bool wsetup){
  if(!prefs->getBool(WIFI, true) || wifi_ssid.length() == 0 || wifi_pwd.length() == 0) return false;

  if(WiFi.isConnected()){
    if(wsetup && WiFi.SSID() != wifi_ssid) {
      WiFi.disconnect();
    }else return true;
    //log("WiFi connected to ssid: %s, ip:%s", wifi_ssid.c_str(),  WiFi.localIP().toString().c_str());
  }
    
  log("Connecting to WiFi ssid: %s", wifi_ssid.c_str());
  WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){ this->onNetEvent(event);});
  WiFi.persistent(true);
  WiFi.setAutoReconnect(true);

  wifiSt = WiFi.begin(wifi_ssid, wifi_pwd);
  if(!wifiSt){
    Utils::buzzer(3);
    log("WiFi connection failed - SSID %s, status: %s", wifi_ssid.c_str(), wifiStatus());
    return false;
  }

  //log("Connected to WiFi SSID %s IP address: %s", wifi_ssid.c_str(), WiFi.localIP().toString().c_str());

  /* if (MDNS.begin("rapidomize") {
    Serial.println(F("MDNS responder started");
  } */
  
  if(wsetup){
    int wsretry = 0;
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      /* wifiStatus();
      if(retry++ % 5 == 0)
        log("Trying to connect to WiFi ssid: %s, status: %s", wifi_ssid.c_str(), wifiStStr.c_str()); */

      if(wsretry++ > 20) {
        Utils::buzzer(3);
        wsretry = 0;
        //return immediately
        log("WiFi connection failed - SSID %s, status: %s", wifi_ssid.c_str(), wifiStatus());
        if(wsetup) return false;
      }
      yield();
      delay(500);
    }

    prefs->putString("wifi_ssid", wifi_ssid);
    prefs->putString("wifi_pwd", wifi_pwd);
    save();
  }

  return true;
}

bool ConProvider::canConnectMQTT(){
  /* _log("hasSetup %d, %d, %d, %s://%s:%d, clientId: %s, username: %s, password: %s, topic: %s", hasSetup, iswsetup, mqttClient->connected(),
              tls?"ssl":"mqtt", host.c_str(), port, clientId.c_str(), username.c_str(), password.c_str(), topic.c_str()); */
  return host.length() != 0 && port > 0 && clientId.length() != 0 
      && username.length() != 0 && password.length() != 0 && topic.length() != 0;// && ver.length() != 0 && qos >= 0;
}

//connect to MQTT server
bool ConProvider::connectMQTT(bool wsetup){
    hasSetup = false;
    iswsetup = wsetup;

    if(!canConnectMQTT()){
      log("MQTT connection failed! Invalid Connection details: %s, host: %s , port: %d, clientId: %s, username: %s, password: %s, topic: %s", 
             tls?"TLS":"", host.c_str(), port, clientId.c_str(), username.c_str(), password.c_str(), topic.c_str());
      iswsetup = false;
      return false;
    } 

    int wsretry = 0;
    while(!isEthCon && !isWifiCon){
      /* if(wsretry++ % 5 == 0)
        log("Trying to establish network connection"); */

      if(wsretry++ > 20) {
        Utils::buzzer(3);
        wsretry = 0;
        //return immediately
        log("Network connection is failed!");
        return false;
      }
      yield();
      delay(500);
    }

    /* if(WiFi.status() != WL_CONNECTED){
      connectETH(wsetup);
      //connectWiFi(wsetup);
    } */

    int retry = 0, cnt = 0;
    if(netClient){
      if(netClient->connected())  netClient->clear();
      if(wsetup && mqttClient->connected()) mqttClient->disconnect();

      if(tls && !wastls || !tls && wastls){
        netClient->stop(); //if already connected, but change for different settings
        delete netClient;
        netClient = nullptr;
      }
    } 

    if(!netClient){
      if(tls) {
        NetworkClientSecure *net = new NetworkClientSecure();
        if(host == RPZ_PL) net->setCACert(RPZ_CA_CERT);
        else if(CA_CERT) net->setCACert(CA_CERT);
        else net->setInsecure(); //FIXME: not working
        netClient = net;
        wastls = true;
      }else{
        wastls = false;
        netClient = new NetworkClient();
      } 
    }
    mqttClient->setClient(*netClient);

    if(wsetup || !netClient->connected()){
      yield();

      log("Connecting to MQTT broker: %s://%s:%d", tls?"ssl":"mqtt", host.c_str(), port);
      netClient->stop();

      int err;
      cnt = 0;
      while(!(err = netClient->connect(host.c_str(), port, timeout * 1000))){
        yield();
        if(!isEthCon && !isWifiCon){
          log("Network connection is failed!");
          Utils::buzzer(3);
          iswsetup = false;
          return false;
        }
        if(wsetup) {
          //return immediately
          log("Failed connecting to MQTT broker: %s://%s:%d", tls?"ssl":"mqtt", host.c_str(), port);
          iswsetup = false;
          return false;
        }
        /* if(cnt++ > 1) {//Unfortunately, TLS connection is sloooow on ESP32 
          cnt = 0;
        } */
        log("Failed connecting to MQTT broker: %s://%s:%d, ...retrying", tls?"ssl":"mqtt", host.c_str(), port);
        Utils::buzzer(1);
        delay(200);
      }
    }

    log("Initiating MQTT communication with broker: %s://%s:%d with ClientID: %s, Username: %s, Password: xxxxxx, topic: %s", 
             tls?"ssl":"mqtt",  host.c_str(), port, clientId.c_str(), username.c_str(), topic.c_str());//password.c_str()
    
    wsretry = 0, cnt = 0;
    //mqttClient->setSocketTimeout(MQTT_SOCKET_TIMEOUT);
    //mqttClient->setSocketTimeout(2);
    if(!mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str())){//internal check is done to  see connected()
      Utils::buzzer(2);
      log("Failed establishing MQTT communication with broker: %s://%s:%d, with ClientID: %s Username: %s Password: xxxxxx...Check Credentials.", 
        tls?"ssl":"mqtt", host.c_str(), port, clientId.c_str(), username.c_str());
      iswsetup = false;  
      return false;
    }

    log("Successfully initiated communication with MQTT broker:  %s://%s:%d", tls?"ssl":"mqtt", host.c_str(), port);
    //mqttClient->setSocketTimeout(MQTT_SOCKET_TIMEOUT);
    mqttClient->setKeepAlive(90);
    // subscribe to a topic:
    //log("Subscribing to topic: %s", subtopic);
    //mqttClient->subscribe(subtopic);

    hasSetup = true;
    iswsetup = false;
    return true;
}

bool ConProvider::netConnect(bool wsetup){
  return true;
}

void ConProvider::onNetEvent(arduino_event_id_t event){
  if(lstEv == event) return;

  lstEv = event;
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      //log("Ethenet Started");
      // The hostname must be set after the interface is started, but needs
      // to be set before DHCP, so set it from the event handler thread.
      ETH.setHostname(RPZC_HOST);
      break;
    case ARDUINO_EVENT_ETH_CONNECTED: 
      log("Ethenet - Connected, waiting for IP ..."); 
      break;
    case ARDUINO_EVENT_ETH_GOT_IP6: 
      log("Ethenet IPv6 %s", ETH.linkLocalIPv6().toString().c_str());
      isEthCon = true;
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      log("Ethenet IPv4 %s", ETH.localIP().toString().c_str());
      isEthCon = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
    case ARDUINO_EVENT_ETH_DISCONNECTED:
    case ARDUINO_EVENT_ETH_STOP:
      isEthCon = false;
      log("Ethenet - Disconnected");
      if(!isWifiCon) Utils::buzzer(3);
      break;

    //WiFi
    case ARDUINO_EVENT_WIFI_AP_START:
      log("WiFi Access Point %s - Started", AP_SSID.c_str());
      WiFi.softAPsetHostname(AP_SSID.c_str());
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP: 
      log("WiFi Access Point %s - Stopped", AP_SSID.c_str()); 
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      isWifiCon = false;
      WiFi.setHostname(RPZC_HOST);
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED: 
      isWifiCon = false;
      log("WiFi SSID %s - Connected, waiting for IP ...", wifi_ssid.c_str()); 
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
      isWifiCon = true;
      log("WiFi SSID %s, IPv6: %s", wifi_ssid.c_str(), WiFi.linkLocalIPv6().toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      isWifiCon = true;
      log("WiFi SSID %s, IPv4: %s", wifi_ssid.c_str(), WiFi.localIP().toString().c_str());
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: 
    case ARDUINO_EVENT_WIFI_STA_STOP:   
      isWifiCon = false;
      log("WiFi SSID %s - Disconnected", wifi_ssid.c_str()); 
      if(!isEthCon) Utils::buzzer(3);
      break;
    
    default: break;
  }
} 

void ConProvider::netInit(){
  Serial.println("Init WiFi...");
  bool bap = prefs->getBool(AP, true);
  if(!bap){
    WiFi.mode(WIFI_STA);   
  }else{
    WiFi.mode(WIFI_AP_STA);   
  }

  scan();  

  //start Ethernet if available
  Network.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){ this->onNetEvent(event);});
  ethSt = ETH.begin();

  /*if (!LittleFS.begin()) {
    LittleFS.format();
    LittleFS.begin();
  } */

  //provide page for config or reconfig
  if(wifi_ssid.length() == 0 || wifi_pwd.length() == 0){
    delay(100);
    wifi_ssid.clear();
    wifi_pwd.clear();

    WiFi.softAP(AP_SSID.c_str(), NULL);
    
    Serial.printf("Created AP %s with IP address: %s\n", AP_SSID.c_str(), WiFi.softAPIP().toString().c_str());
  }else{
    connectWiFi();
  }

}

void ConProvider::init(PubSubClient *mqttClient, Peripheral **peripherals, Preferences *prefs){

  this->mqttClient = mqttClient;
  this->peripherals = peripherals;
  this->prefs = prefs;

  wifi_ssid = prefs->getString("wifi_ssid", "");
  wifi_pwd = prefs->getString("wifi_pwd", "");

  host = prefs->getString("host", RPZ_PL);
  port = prefs->getShort("port", 8883);
  clientId = prefs->getString("clientId", "cid");
  username = prefs->getString("username", "");
  password = prefs->getString("password", "");
  topic = prefs->getString("topic", "");
  tls = prefs->getBool("tls", true);
  ver = prefs->getString("ver", "3.1.1");
  qos = prefs->getUInt("qos", 0); 
  rpzfmt = prefs->getBool("rpzfmt", true);

  netInit();

  //SSE
  events.onConnect([this](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("SSE client connected! Last message ID is: %u\n", client->lastId());
    }
    //send event with message "Connected!", id current millis
    // and set reconnect delay to 1 second
    client->send(".", NULL, millis(),1000);
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
  server.on("/auth", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->onAuth(request);
  }, NULL, aggregator);
  server.on("/logout", HTTP_POST, [this](AsyncWebServerRequest *request) {
    authst = false;
    disableTimer(false);
    request->send(200, "application/json", "{\"url\":\"/\"}");
  }, NULL, aggregator);
  server.on("/prefs", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if(!checkAuth(request)) return;
    this->onPrefs(request);
  }, NULL, aggregator);
  server.on("/wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if(!checkAuth(request)) return;
    this->onWifi(request);
  }, NULL, aggregator);
  server.on("/mqtt", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if(!checkAuth(request)) return;
    this->onMqtt(request);
  }, NULL, aggregator);
  server.on("/peri", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if(!checkAuth(request)) return;
    this->onPeri(request);
  }, NULL, aggregator);
  server.on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if(!checkAuth(request)) return;
    this->onReset(request);
  }, NULL, aggregator);
  
  server.on("/fwurl", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if(!checkAuth(request)) return;
    this->onFwUrl(request);
  }, NULL, aggregator);

  server.on("/fwfile", HTTP_POST, [this](AsyncWebServerRequest *request) {
    if(!checkAuth(request)) return;
    if (request->getResponse()) {
      Serial.println("response already created");
      return;
    }
    
    if(fwupdated) {
      fwupdated = false;
      log("Firmware image uploaded successfully! Device will be rebooted shortly to start new firmware");
      request->send(200, "application/json", "{\"err\":\"Firmware image uploaded successfully! Device will be rebooted shortly to start new firmware!\"}");
      //delay(500);
      this->onUpgrade(request);
      return;
    }
    log("Firmware image upload failed");
    request->send(400, "application/json", "{\"err\":\"Firmware image upload failed...!\"}");
  },
  [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if(!index && !checkAuth(request)) return;
    this->onFwFile(request, filename, index, data, len, final);  
  });

  server.begin();
}

}
