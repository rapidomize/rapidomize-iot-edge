#ifndef RPZ_WIFI_H_
#define RPZ_WIFI_H_

#include <Preferences.h>
#include <AsyncTCP.h>
#include <NetworkClient.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

#include "peripherals/peripheral.h"
#include "utils.h"

namespace rpz{

//connection & config provider
class ConProvider: public Utils{
    public: 
        ConProvider(): server(80), events("/evts"){
            server.addHandler(&events);
        }

        void init(PubSubClient *mqttClient, Peripheral **peripherals, Preferences *prefs);
        bool connectMQTT(bool wsetup=false);
        bool canConnectMQTT();
        

        bool hasSetup = false;
        
        //mqtt
        String host; 
		short port; 
        String clientId;
        String username;
        String password;
        String topic;
        bool tls;
        String ver;
        uint8_t qos;
        int timeout = 5;  //seconds
        bool rpzfmt = true; //Enable/Disable Rapidomize platform message format
        bool wastls = false; //if we were using secure net client
        bool iswsetup = false;

    private:
        void netInit();
        bool connectWiFi(bool setup=false);
        bool connectETH(bool setup=false);
        bool netConnect(bool setup=false);
        void scan();

        void homePage(AsyncWebServerRequest *request, int status=200, const char *err=nullptr);
        void err(const char *err);
        char * getDash();
        char * getWifi();
        char * getMqtt();
        char * getPeri();
        //char * getFile(const char * name);
        void onAuth(AsyncWebServerRequest *request);
        bool checkAuth(AsyncWebServerRequest *request);
        void onWifi(AsyncWebServerRequest *request);
        void onMqtt(AsyncWebServerRequest *request);
        void onPeri(AsyncWebServerRequest *request);
        void onPrefs(AsyncWebServerRequest *request);
        void onReset(AsyncWebServerRequest *request);
        void restart(AsyncWebServerRequest *request);
        void onFwUrl(AsyncWebServerRequest *request);
        void onFwFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        void onUpgrade(AsyncWebServerRequest *request);

        void onNetEvent(arduino_event_id_t event);
        void toJson(AsyncWebServerRequest *request, JsonDocument &doc);

        const char * wifiStatus();

        void initlog(){
             Utils::ev = &events;
        }

        void save();

        bool fwupdated = false;
        String ssids[20];
        uint8_t ssid_cnt;
        String wifi_ssid;
        String wifi_pwd;
        String wifiStStr;
        bool ethSt;
        bool wifiSt;
        arduino_event_id_t lstEv;
        bool isEthCon = false;
        bool isWifiCon = false;
        
        AsyncEventSource events;
        AsyncWebServer server;
        NetworkClient *netClient = nullptr;
        PubSubClient *mqttClient = nullptr;
        Preferences *prefs = nullptr;
        Peripheral **peripherals = nullptr;
};

}

#endif  //RPZ_WIFI_H_