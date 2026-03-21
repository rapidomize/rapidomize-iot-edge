#include <Arduino.h>
#include <Update.h>
// #include <ESPmDNS.h>

#include <cstring>

#include <NetworkClientSecure.h>
// #include <NetClientSecure.h>

#include "provider.h"
#include "device.h"
#include "utils.h"
#include "tmpl.h"

namespace rpz{

const char * AP_SSID="iot_edge";
const char *RPZ_PL = "ics.rapidomize.com";


//ISRG Root X1
const char *RPZ_CA_CERT = R"(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAwTzELMAkGA1UE
BhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNoIEdyb3VwMRUwEwYDVQQD
EwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQG
EwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMT
DElTUkcgUm9vdCBYMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54r
Vygch77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+0TM8ukj1
3Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6UA5/TR5d8mUgjU+g4rk8K
b4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sWT8KOEUt+zwvo/7V3LvSye0rgTBIlDHCN
Aymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyHB5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ
4Q7e2RCOFvu396j3x+UCB5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf
1b0SHzUvKBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWnOlFu
hjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTnjh8BCNAw1FtxNrQH
usEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbwqHyGO0aoSCqI3Haadr8faqU9GY/r
OPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CIrU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4G
A1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY
9umbbjANBgkqhkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ3BebYhtF8GaV
0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KKNFtY2PwByVS5uCbMiogziUwt
hDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJw
TdwJx4nLCgdNbOhdjsnvzqvHu7UrTkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nx
e5AW0wdeRlN8NwdCjNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZA
JzVcoyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq4RgqsahD
YVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPAmRGunUHBcnWEvgJBQl9n
JEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57demyPxgcYxn/eR44/KJ4EBs+lVDR3veyJ
m+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";

// CA Certificates esp. for local servers (if using self-signed or specific CA)
const char *CA_CERT  = R"(
-----BEGIN CERTIFICATE-----
MIIDxDCCAqygAwIBAgIULKxmZYFpCmR4GV/q+DpoAwXGb8gwDQYJKoZIhvcNAQEL
BQAwaTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMRIwEAYDVQQHDAlDYWxpZm9u
aWExFzAVBgNVBAoMDlJhcGlkb21pemUgTExDMQwwCgYDVQQLDANFbmcxEjAQBgNV
BAMMCTEwLjEuMS4xMDAeFw0yNjAzMDgxNTMzMDBaFw0zNjAzMDUxNTMzMDBaMGkx
CzAJBgNVBAYTAlVTMQswCQYDVQQIDAJDQTESMBAGA1UEBwwJQ2FsaWZvbmlhMRcw
FQYDVQQKDA5SYXBpZG9taXplIExMQzEMMAoGA1UECwwDRW5nMRIwEAYDVQQDDAkx
MC4xLjEuMTAwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCqG3qDCzs6
TH7RdkHkaNIlV4mW+HlVJAtOD3jXdwXGEmt7jVH+HuCRjokr0PKYOcS2KknlAamH
YHVZhCddGrzQYZH4kKWL19FCeu+Y0Wf/BmEb1osn6qiSRfYyFR85nCXEalUDkwAp
qY+W6XY374X89XS9XSo2hl0wV7dyt4EBCRZEoh4FCsF4ekOnvah0PGpTu54/V1xQ
3XA/mOXJQucA3DUkrPPY8QRF4FMVTnZw8Tsx5aVYdIZgdQUWPvCZPp7wd4zHDCc8
u9K4ZjuvQiOQvv/OAKURtVUBot6VhjCo2vchAjIhE9uGQDoTEtCHcAV9C3Nd1E14
Y96+Rl19CAuBAgMBAAGjZDBiMB0GA1UdDgQWBBSD656r8chf/RZVlFUHtZytonsG
UTAfBgNVHSMEGDAWgBSD656r8chf/RZVlFUHtZytonsGUTAPBgNVHRMBAf8EBTAD
AQH/MA8GA1UdEQQIMAaHBAoBAQowDQYJKoZIhvcNAQELBQADggEBAEaHOstULWLY
SzzaJszqkMrjRzxwEFyBNBoe2qfn+8QLR9df3nUiB0EYEGza2FOTqflNN6Vx5v93
pqGiowv51E/A9O4DJ9wwDvZyLR1oXJBra4pUl1gXGz0FJTDzIaRtUBpJx6Z6NXiY
YRs9Z8Z4goI2sJYI48+bxLvYtQc+rToeA6dtSQ2LbUwa/njFxokb7Y2kbBUupC7v
+RoK1AzX6MPYEKgAaeJ+Z8CA1FaPwH42d3u4uKZv2T+eyGzpZXL5mlymphKB4NVv
BPi+AFmAMY6smE+io26vRQGEJmUA5rRAy+DnTw1l3gY6aXB5jUhEqBHSGeltb5F+
v3hMMocP6fo=
-----END CERTIFICATE-----
)";


static const int PG_SIZE = 1024*32; //is this too large?
static const int TAB_SIZE = 1024*4;

/* 
  Connect to the WiFi SSID - 'iot_edge'
  Go to http://192.168.4.1 in a web browser to configure the gateway
*/      
void ConProvider::homePage(AsyncWebServerRequest *request, int status, const char *err) {
  Serial.printf(PSTR("Page Loading - %d - %s\n"), status, err?err:"");
  //yield();
  
  char *dash = getDash();
  char *wifi = getWifi();
  char *mqtt = getMqtt();
  char *peri = getPeri();

  char *page = (char *) malloc(PG_SIZE);
  if(wifi_ssid.length() == 0 || wifi_pwd.length() == 0){
    sprintf(page, main_tmpl, err?err:"", dash, wifi, "", "", RPZ_VERSION);
  }else{
    sprintf(page, main_tmpl, err?err:"", dash, wifi, mqtt, peri, RPZ_VERSION);
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
  char *fr = (char *) malloc(TAB_SIZE);
  sprintf(fr, dash_tmpl, "rpz-d2x2t2ux-we", RPZ_VERSION, ESP.getCpuFreqMHz(), ESP.getSketchSize()/1024, WiFi.localIP().toString().c_str(), "");
  return fr;
}

char * ConProvider::getWifi(){
  String ssidlst;
  char *fr = (char *) malloc(TAB_SIZE);
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
  char *fr = (char *) malloc(TAB_SIZE);
  sprintf(fr, mqtt_tmpl, host.c_str(), port, tls?"checked":"", clientId.c_str(), username.c_str(), "xxxxxxxx", topic.c_str(), ver.c_str(), qos, rpzfmt?"checked":"");
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
      request->send(200, "application/json", "{}");
      return;
    }     
  }
  request->send(400, "application/json", "{\"err\":\"Error connecting to WiFi! Check SSID/Password!\"}");
  //homePage(request, 400, "Error connecting to WiFi! Check SSID/Password!");
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
}

const char * ConProvider::wifiStatus(){
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL: status =  "WL_NO_SSID_AVAIL"; break;
      case WL_CONNECT_FAILED:  status = "WL_CONNECT_FAILED. Check SSID/Password!"; break;
      case WL_CONNECTION_LOST:  status = "WL_CONNECTION_LOST"; break;
      case WL_DISCONNECTED:  status = "WL_DISCONNECTED";  break;
      default:
        break;
    }
    return status.c_str();
}

bool ConProvider::connectWiFi(bool wsetup){
    if(WiFi.isConnected() && WiFi.SSID() == wifi_ssid ||
        wifi_ssid.length() == 0 || wifi_pwd.length() == 0) return false;
    
    log(PSTR("Connecting to WiFi ssid: %s, pwd: xxxxxx"), wifi_ssid.c_str());//, wifi_pwd.c_str());

    WiFi.disconnect(true);  //disconnect from wifi AP to set wifi connection in STA
    //WiFi.mode(WIFI_AP_STA);    //init wifi mode
    WiFi.begin(wifi_ssid, wifi_pwd);
    int retry = 0;
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      log(PSTR("Cannot connect to WiFi, status: %s"), wifiStatus());

      if(retry++ > 10) {
        Utils::buzzer(3);
        retry = 0;
        //return immediately
        if(wsetup) return false;
      }
      yield();
      delay(500);
    }

    log(PSTR("Connected to WiFi SSID %s IP address: %s"), wifi_ssid.c_str(), WiFi.localIP().toString().c_str());

    /* if (MDNS.begin("rapidomize")) {
      Serial.println(F("MDNS responder started"));
    } */

    prefs->putString("wifi_ssid", wifi_ssid);
    prefs->putString("wifi_pwd", wifi_pwd);
    save();

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

    if(WiFi.status() != WL_CONNECTED){
      connectWiFi(wsetup);
    }

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
        if(WiFi.status() != WL_CONNECTED){
          log(PSTR("WiFi is not connected, status: %s"), wifiStatus());
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
        if(cnt++ > 5) {//Unfortunately, TLS connection is sloooow on ESP32
          log("Failed connecting to MQTT broker: %s://%s:%d, ...retrying", tls?"ssl":"mqtt", host.c_str(), port);
          Utils::buzzer(1);
          cnt = 0;
        }
        delay(200);
      }
    }

    log("Initiating MQTT communication with broker: %s://%s:%d with ClientID: %s, Username: %s, Password: xxxxxx, topic: %s", 
             tls?"ssl":"mqtt",  host.c_str(), port, clientId.c_str(), username.c_str(), topic.c_str());//password.c_str()
    
    retry = 0, cnt = 0;
    //mqttClient->setSocketTimeout(MQTT_SOCKET_TIMEOUT);
    //mqttClient->setSocketTimeout(2);
    if(!mqttClient->connect(clientId.c_str(), username.c_str(), password.c_str())){//internal check is done to  see connected()
      Utils::buzzer(2);
      log("Failed initiating MQTT communication with broker: %s://%s:%d, with ClientID: %s Username: %s Password: xxxxxx...Check Credentials.", 
        tls?"ssl":"mqtt", host.c_str(), port, clientId.c_str(), username.c_str());
      iswsetup = false;  
      return false;
    }

    log("Successfully connected to MQTT broker:  %s://%s:%d", tls?"ssl":"mqtt", host.c_str(), port);
    //mqttClient->setSocketTimeout(MQTT_SOCKET_TIMEOUT);
    mqttClient->setKeepAlive(90);
    // subscribe to a topic:
    //log(PSTR("Subscribing to topic: %s"), subtopic);
    //mqttClient->subscribe(subtopic);

    hasSetup = true;
    iswsetup = false;
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

  scan();

  //SSE
  events.onConnect([this](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("SSE client connected! Last message ID is: %u\n", client->lastId());
    }
    //send event with message "Connected!", id current millis
    // and set reconnect delay to 1 second
    client->send(">", NULL, millis(),1000);
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

    const char * url = (const char *)doc["fw_url"];
    if(!url){
      Serial.println("URL Error: Null");
      request->send(400, "application/json", "{\"err\":\"Invalid firmware download url?\"}");
      return;
    }
    bool value = http.begin(*this->netClient, url);
    Serial.printf("%d", value);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        // Get the stream pointer to the firmware data
        NetworkClient* stream = http.getStreamPtr();

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
       const char *msg = "Firmware update successfully! Device will be rebooted shortly to start new firmware";
        log(msg);
        request->send(200, "application/json", "{\"err\":\"Firmware update successfully! Device will be rebooted shortly to start new firmware!\"}");
        //delay(500);
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
        //delay(500);
        this->onUpgrade(request);
        return;
      }
      request->send(400, "application/json", "{\"err\":\"Firmware image upload failed...!\"}");
  },
  [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
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
          log("Firmware image upload file: %s\n", filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
            Update.printError(Serial);
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
        log("Firmware written successfully - file %s\n", filename.c_str());
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
    server.begin();
  }else{
    connectWiFi();
    server.begin();
    //connectMQTT();
  }
}

}