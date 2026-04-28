#include <Arduino.h>

#include <string>
#include <functional>
#include <memory>


#include "device.h"
#include "utils.h"
#include "peripherals/peripheral.h"
#include "peripherals/pzem01x.h"
#include "peripherals/sht3x.h"
#include "peripherals/din.h"
// #include "peripherals/relay.h"
#include "peripherals/ahtx0.h"
#include "peripherals/bmp280.h"

#include <ArduinoJson.h>



rpz::Device device;

namespace rpz{

const bool DEBUG=true;
const int  MAX_DATA=1024;

const char *STATE_MSG = R"({"msg":%s,"op":2})";
//FIXME: format
const char *ACK_MSG = R"({"msg":{"n":"circuit", "v":%d},"op":2})";

void Device::init(){
    if (!prefs.begin("rpzc", false)) {
        Serial.println("Failed to initialize NVS. Restarting...");
        ESP.restart();
    }

    memset(peripherals, 0, sizeof(Peripheral*) * MAX_PERIPHERALS);
    //std::fill(peripherals, peripherals+MAX_PERIPHERALS, static_cast<Peripheral*>(nullptr));
    peripherals[pcnt++] = new PZEM01x(&prefs, &conprv);
    peripherals[pcnt++] = new Sht3x(&prefs, &conprv);
    peripherals[pcnt++] = new DIN(&prefs, &conprv);
    peripherals[pcnt++] = new DIN(&prefs, &conprv, 2);
    peripherals[pcnt++] = new AHTx0(&prefs, &conprv);
    peripherals[pcnt++] = new BMP280(&prefs, &conprv);  
    //TODO:
    /* peripherals[senscnt++] = new Relay(&prefs);
    peripherals[senscnt++] = new Relay(&prefs,2); */
    for(int i=0; i < MAX_PERIPHERALS && peripherals[i]; i++){
        peripherals[i]->init(nullptr);
    }

    pinMode(LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    conprv.init(&mqttClient, peripherals, &prefs);
    Serial.println("IoT Edge is ready to connect");
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
    mqttClient.publish(topic, msg);
}

void Device::update(){
    Serial.print(".");
    yield();

    while((conprv.hasSetup || conprv.canConnectMQTT()) 
            && !conprv.iswsetup && !mqttClient.connected()) {
        conprv.log("MQTT is not connected, retrying...");
        if(conprv.connectMQTT()) break;
        delay(100);
        Serial.print("*");
        yield();
    }
    
    if(conprv.hasSetup){
        mqttClient.loop();

        String sjreq = "{";
        int cnt=0;
        //report per defined interval
        const unsigned long now = millis();
        bool read = now - prev >= interval;
        // save the last time a message was read
        if (read) prev = now;
        
        for(int i = 0; i < MAX_PERIPHERALS && peripherals[i]; i++){
            char *data = nullptr;
            if(peripherals[i]->isr){//&& peripherals[i]->hasev()
                data = peripherals[i]->read();
            }else{
                //report per defined interval
                if (read) {                    
                    // Serial.printf(PSTR("%s reading data.\n"), peripherals[i]->name);
                    data = peripherals[i]->read();
                }
            }
            if(data){
                if(cnt > 0){
                    sjreq+=", ";
                }
                //Serial.printf(PSTR("\ndata: %s, len: %d\n"), data, ssz);
                sjreq+=data;
                cnt++;
            }            
        }
        if(cnt > 0){
            sjreq+="}";
            char *data = (char *) sjreq.c_str();
            char msg[strlen(data) + 32];
            if(conprv.rpzfmt){
                sprintf(msg, STATE_MSG, data);
                data = msg;
            }
            send(data, conprv.topic.c_str());
        }
    }
}

const char * Device::gettime(){
    uint32_t start = millis();
    time_t now;
    while((millis()-start) <= 5000) {
        time(&now);
        localtime_r(&now, &timeinfo);
        Serial.println(timeinfo.tm_year);
        if(timeinfo.tm_year > (2016 - 1900)){
            break;
        }
        delay(10);
        yield();
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    return nullptr;
}

}

