#include <Arduino.h>

#include <string>
#include <functional>
#include <memory>

#include "device.h"
#include "utils.h"
#include "peripherals/peripheral.h"
#include "peripherals/pzem01x.h"
#include "peripherals/sht3x.h"
#include "peripherals/relay.h"
#include "peripherals/switch.h"
#include "peripherals/ahtx0.h"
#include "peripherals/bmp280.h"

#include <ArduinoJson.h>


rpz::Device device;

namespace rpz{

const bool DEBUG=true;
const int  MAX_DATA=1024;

const char *STATE_MSG = R"({"msg":{"_evt":"circuit", "state":%d},"op":2})";
//FIXME: format
const char *ACK_MSG = R"({"msg":{"n":"circuit", "v":%d},"op":2})";

const char *PERIPHERALS = R"({"msg":{"n":"circuit", "v":%d},"op":2})";

void Device::init(){
    if (!prefs.begin("rpzc", false)) {
        Serial.println("Failed to initialize NVS. Restarting...");
        ESP.restart();
    }

    memset(peripherals, 0, sizeof(Peripheral*) * MAX_PERIPHERALS);
    //std::fill(peripherals, peripherals+MAX_PERIPHERALS, static_cast<Peripheral*>(nullptr));
    peripherals[pcnt++] = new PZEM01x(&prefs);
    peripherals[pcnt++] = new Sht3x(&prefs);
    peripherals[pcnt++] = new Switch(&prefs);
    peripherals[pcnt++] = new Switch(&prefs,2);
    peripherals[pcnt++] = new AHTx0(&prefs);
    peripherals[pcnt++] = new BMP280(&prefs); 
    //TODO:
    /* peripherals[senscnt++] = new Relay(&prefs);
    peripherals[senscnt++] = new Relay(&prefs,2); */
    for(int i=0; i < MAX_PERIPHERALS && peripherals[i]; i++){
        peripherals[i]->init(nullptr);
    }

    pinMode(LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    conprv.init(&mqttClient, peripherals, &prefs);
    Serial.println(F("IoT Edge is ready to connect"));
}


//we should create topic & payload copy of the values if they are required after the callback returns
void Device::recv(const char* topic, byte* payload, unsigned int length) {
    // we received a message, print out the topic and contents
    Serial.printf(PSTR("Received topic: %s, len: %d, payload: %s\n"), topic, length, payload);

    //{"msg":{"n":"circuit","v":1,"dt":"boolean","rwx":"","op":1},"mid":3,"op":1,"uri":"1092720020131081d33f4-7-251a5cdc6b9df"}
    JsonDocument doc;
    auto err = deserializeJson(doc, payload);
    if(err){
        return;
    }
    
    JsonObject msg = doc["msg"];
    Serial.printf("msg => %s\n", (const char* )msg["n"]);
    int op = (int) doc["op"];
    Serial.printf("op: %d\n", op);
    switch (op){
        case RPZ_READ: break;
        case RPZ_WRITE:{
            auto v = 10;//msg["v"];
            const char* dt = (const char* )msg["dt"];
            Serial.printf("dt: %s\n", dt);

            if(strcmp(dt, "boolean") == 0){
                Serial.printf("vb: %d\n", (int)v);
            }else if(strcmp(dt, "double") == 0){
                Serial.printf("vd: %f\n", (double)v);
            }else if(strcmp(dt, "integer") == 0){
                Serial.printf("vi: %d\n", (long)v);
            }
            if(v == 0){
                device.circuit = false;
                // digitalWrite(D3, HIGH);
            }else{
                device.circuit = true;
                // digitalWrite(D3, LOW);
            }
        } break;
        case RPZ_EXEC: break;
        
        default:
            break;
    }
    const char* uri = doc["uri"];
       
    char acktopic[256];
    sprintf(acktopic, "ack", uri);
    char jmsg[128];
    sprintf(jmsg, ACK_MSG, 0);
    device.send(jmsg, acktopic);
    
    /* size_t loc = incoming.find("msg");
    if(loc > 0){
        loc = incoming.find("\"n\"", loc);
        if(loc > 0){
            loc = incoming.find("\"", loc+3);
            if(loc > 0){
                size_t en = incoming.find("\"", loc+1);
                if(en > 0){
                    std::string sen = incoming.substr(loc , en);
                    loc = incoming.find("\"op\"", en);
                }
            }
        }
    } */
}

void Device::send(const char* msg, const char* topic) {
    
    if (!mqttClient.connected() && !conprv.connectMQTT()) {
        conprv.log("MQTT is not connected, unable to send msg %s", msg);
        return;
    }
    mqttClient.loop();//additional keep alive
    conprv.log(msg);
}

void Device::update(){
    Serial.print(".");
    //conprv.events.send("my event content");//,"myevent",millis());
    if(conprv.hasSetup){
        if (!mqttClient.connected() && !conprv.connectMQTT()) {
            conprv.log("MQTT is not connected, retrying...");
            return;
        }
        mqttClient.loop();

        char jreq[MAX_DATA]; jreq[0]='[';jreq[1]='\0';
        int size=1;
        int cnt=0;
        for(int i = 0; i < MAX_PERIPHERALS && peripherals[i]; i++){
            if(peripherals[i]->isr){//&& peripherals[i]->hasev()
                char *data = peripherals[i]->read();
                if(data){
                    if(cnt > 0){
                        jreq[size++]=','; jreq[size] = '\0';
                    }
                    int ssz = strlen(data);
                    //Serial.printf(PSTR("\ndata: %s, len: %d\n"), data, ssz);
                    
                    strlcat(jreq, data, MAX_DATA);
                    size += ssz;
                    cnt++;
                }
            }
        }
        if(cnt > 0){
            jreq[size++]=']'; jreq[size] = '\0';
            send(jreq, conprv.topic.c_str());
        }

        //report per defined interval
        const unsigned long now = millis();
        if (now - prev >= interval) {
            // save the last time a message was sent
            prev = now;

            jreq[0]='[';jreq[1]='\0';
            size=1;
            cnt=0;
            for(int i = 0; i < MAX_PERIPHERALS && peripherals[i]; i++){
                if(peripherals[i]->isr) continue; //skip if isr

                //Serial.printf(PSTR("%s reading data.\n"), peripherals[i]->name);
                //delay(1000);
                char *data = peripherals[i]->read();
                if(data){
                    if(cnt > 0){
                        jreq[size++]=','; jreq[size] = '\0';
                    }
                    int ssz = strlen(data);
                    //Serial.printf(PSTR("\ndata: %s, len: %d\n"), data, ssz);
                    
                    strlcat(jreq, data, MAX_DATA);
                    size += ssz;
                    cnt++;
                }
            }
            if(cnt > 0){
                jreq[size++]=']'; jreq[size] = '\0';
                send(jreq, conprv.topic.c_str());
            }
        }
    }

    
    /* if(indicate++ > 10){
        Utils::indicate();
        indicate = 0;
    } */
}

}

