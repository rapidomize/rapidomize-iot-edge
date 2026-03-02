#ifndef RPZ_WIFI_H_
#define RPZ_WIFI_H_

#include <Preferences.h>
#include <AsyncTCP.h>
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
        ConProvider(): server(80), host("ics.rapidomize.com"), port(8883), events("/evts"){
            server.addHandler(&events);
        }

        void init(PubSubClient *mqttClient, Peripheral **peripherals, Preferences *prefs);
        bool connectMQTT(bool setup=false);

        bool haswifi = false;
        bool hasSetup = false;
        
        //mqtt
        String host; 
		int port; 
        String clientId;
        String username;
        String password;
        String topic;
        bool tls;
        String ver;
        uint8_t qos;

    private:
        bool connectWiFi(bool setup=false);
        void scan();

        void homePage(AsyncWebServerRequest *request, int status=200, const char *err=nullptr);
        void err(const char *err);
        char * getDash();
        char * getWifi();
        char * getMqtt();
        char * getPeri();
        void onWifi(AsyncWebServerRequest *request);
        void onMqtt(AsyncWebServerRequest *request);
        void onPeri(AsyncWebServerRequest *request);
        
        void onUpgrade(AsyncWebServerRequest *request);
        void toJson(AsyncWebServerRequest *request, JsonDocument &doc);

        void onReset(AsyncWebServerRequest *request);
        void restart(AsyncWebServerRequest *request);

        void initlog(){
             Utils::ev = &events;
        }

        void save();

        bool fwupdated = false;
        String ssids[20];
        uint8_t ssid_cnt;
        String wifi_ssid;
        String wifi_pwd;

        AsyncEventSource events;
        AsyncWebServer server;
        WiFiClient wifiClient; 
        PubSubClient *mqttClient;
        Preferences *prefs;
        Peripheral **peripherals;
};

}

#endif  //RPZ_WIFI_H_