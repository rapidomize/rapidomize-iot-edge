#ifndef RPZ_SHT3X_H_
#define RPZ_SHT3X_H_

#include "i2c.h" 

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>
/* 
    SHT3x Temperature and Humidity Sensor
 */

namespace rpz{

const char *SHT3X_SEN_MSG  = R"({"temperature":%f, "humidity":%f})"; 

class Sht3x: public I2C{

    Adafruit_SHT31 sht3x = Adafruit_SHT31();

    public: 
    Sht3x(Preferences *prefs, int seq=1):I2C(prefs,seq){
        sprintf(name, "SHT3x_%d", seq);

        /* defaults can be overridden here
        conf["SDA"] = 4;
        conf["SCL"] = 16; 
        */
        char buf[10];
        sprintf(buf, "%X", SHT31_DEFAULT_ADDR); //default 0x44 - Set to 0x45 for alternate i2c addr
        conf["I2C_ADDRESS"] = buf; 
        configure();
    }

    void init(JsonDocument *jconf) {
        I2C::init(jconf);

        if (!sht3x.begin(i2caddr)) {   
            Serial.printf(PSTR("%s cannot initiate a connection SDA: %d,  SCL: %d, Address %X\n"), name, sda, scl, i2caddr);
            return;
        }
        
        inited = true;
        Serial.printf(PSTR("%s initialized.\n"), name);
    }

    char * read(){
        if(!inited) return nullptr;

        float temp = sht3x.readTemperature();
        float humidity = sht3x.readHumidity();

        if(!isnan(temp) && !isnan(humidity)){
            sprintf(data, SHT3X_SEN_MSG, temp, humidity);
            return data;    
        }   
        Serial.printf("%s failed to read temperature & humidity\n",  name);
        return nullptr;
    }

};

}

#endif