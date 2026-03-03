#ifndef RPZ_AHTX0_H_
#define RPZ_AHTX0_H_

#include "i2c.h" 

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h> 

/* 
    AHTX0 Temperature and Humidity Sensor, e.g. AHT10, AHT20
 */

namespace rpz{


const char *AHTx0_SEN_MSG  = R"({"temperature":%f, "humidity":%f})";    


class AHTx0: public I2C{

    Adafruit_AHTX0 aht;

    public: 
        AHTx0(Preferences *prefs, int seq=1):I2C(prefs,seq){
            sprintf(name, "AHTx0_%d", seq);

            /* defaults can be overridden here
            conf["SDA"] = 4;
            conf["SCL"] = 16; 
            */
            char buf[10];
            sprintf(buf, "%X", AHTX0_I2CADDR_DEFAULT);
            conf["I2C_ADDRESS"] = buf; 
            
            configure();
        }

        void init(JsonDocument *jconf) {
            I2C::init(jconf);
            
            // Initialize sensor
            if (!aht.begin()) {
                Serial.printf(PSTR("%s cannot initiate a connection SDA: %d,  SCL: %d, Address %X\n"), name, sda, scl, i2caddr);
                return;
            }
            inited = true;
            Serial.printf(PSTR("%s initialized.\n"), name);
		}

        char * read(){
            if(!inited) return nullptr;

			// Read data from AHTx0
            sensors_event_t humidity, temp;
            aht.getEvent(&humidity, &temp);

            if(!isnan(temp.temperature) && !isnan(humidity.relative_humidity)){
                sprintf(data, AHTx0_SEN_MSG, temp.temperature, humidity.relative_humidity);
                return data;    
            }  
            Serial.printf("%s failed to read temperature & humidity\n",  name);
            return nullptr;
		}
};

}

#endif