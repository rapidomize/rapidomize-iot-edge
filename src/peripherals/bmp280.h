#ifndef RPZ_BMP280_H_
#define RPZ_BMP280_H_

#include "i2c.h" 

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

/* 
    BMP280 Pressure & Altitude Sensor
 */

namespace rpz{


const char *BMP280_SEN_MSG  = R"({"pressure":%f, "altitude":%f})";    
 
class BMP280: public I2C{

    Adafruit_BMP280 bmp;   

    public: 
        BMP280(Preferences *prefs, int seq=1):I2C(prefs,seq){
            sprintf(name, "BMP280_%d", seq);

            /* defaults can be overridden here
            conf["SDA"] = 4;
            conf["SCL"] = 16; 
            */
            char buf[10];
            sprintf(buf, "%X", BMP280_ADDRESS);
            conf["I2C_ADDRESS"] = buf; 
            configure();
        }

        void init(JsonDocument *jconf) {
            I2C::init(jconf);
            
            // Initialize BMP280 sensor
            if (!bmp.begin(i2caddr, BMP280_CHIPID)) {
                Serial.printf(PSTR("%s cannot initiate a connection SDA: %d,  SCL: %d, Address %X\n"), name, sda, scl, i2caddr);
                return;
            }

            /* Default settings from datasheet. */
            bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,    
                            Adafruit_BMP280::SAMPLING_X2,         /* Temp. oversampling */
                            Adafruit_BMP280::SAMPLING_X16,        /* Pressure oversampling */
                            Adafruit_BMP280::FILTER_X16,          /* Filtering. */
                            Adafruit_BMP280::STANDBY_MS_500);     /* Standby time. */

            inited = true;
            Serial.printf(PSTR("%s initialized.\n"), name);
		}

        char * read(){
            if(!inited) return nullptr;

            // Read data from BMP280
            float pressure = bmp.readPressure() / 100.0F;  // Convert to hPa
            float altitude = bmp.readAltitude(1013.25);    // Adjust for sea level pressure in hPa

            if(!isnan(pressure) && !isnan(altitude)){
                sprintf(data, BMP280_SEN_MSG, pressure, altitude);
                return data;
            }
            Serial.printf("%s Failed to read pressure & altitude\n",  name);
            return nullptr;
		}
};

}

#endif