#ifndef RPZ_SENSOR_H_
#define RPZ_SENSOR_H_

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>


namespace rpz{

struct KV{
    char name[64];
    union{
        int ivalue;
        float fvalue;
        char svalue[100];
    };
};   

class Peripheral{
    public: 
        Peripheral(Preferences *prefs, int seq=1):seq(seq){
            this->prefs = prefs;
        }

        virtual bool configure(){
            String sconf = prefs->getString(name);
            if(sconf.length() != 0){
                auto err = deserializeJson(conf, sconf.c_str());
                if(err){
                    Serial.printf(PSTR("Error parsing configuration for %s : %s\n"), name, sconf.c_str());
                    return false;
                }
            }
            conf["enabled"] = enabled = (bool)conf["enabled"] | false;

            //TODO: debug
            serializeJson(conf, sconf);
            Serial.printf("%s [%d] ==> %s\n", name, enabled?1:0, sconf.c_str());
            
            return true;

        }

        virtual char * confpg() = 0;
        virtual void init(JsonDocument *jconf) {
            if(!jconf || strcmp((const char*)(*jconf)["id"], name) != 0) return;

            conf = *jconf;
            //for now only enable/disable
            if((*jconf)["enabled"])
                conf["enabled"] = enabled = strcmp((const char*)(*jconf)["enabled"], "on") == 0;
            else
                conf["enabled"] = enabled = false;

            String sconf;
            serializeJson(conf, sconf);
            Serial.printf("\nsaving conf %s : %s\n", name, sconf.c_str());
            prefs->putString(name, sconf.c_str());
        }
        virtual char * read(){
			return nullptr;
		}
        virtual void *write(char *data){
			return nullptr;
		};
        virtual void *exec(char *fn){
			return nullptr;
		}

        virtual ~Peripheral() = default;

        bool isr = false;
        char name[60];

    protected:
        Preferences *prefs;
        JsonDocument conf;
        bool enabled = false;
        char data[512];
        int seq;

        bool inited = false;
        //I2C
        uint8_t sda;
        uint8_t scl;
        uint8_t i2caddr;
        //RS485
        uint8_t rxPin;
        uint8_t txPin;
        uint8_t dePin;
        uint32_t baudRate;
        uint8_t rs485addr; //slave address
};

}

#endif /* RPZ_SENSOR_H_ */
