#ifndef RPZ_BMP280_H_
#define RPZ_BMP280_H_

#include "peripheral.h" 

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

/* 
    BMP280 Pressure & Altitude Sensor
 */

namespace rpz{


const char *BMP280_SEN_MSG  = R"({"pressure":%f, "altitude":%f})";    
 
const char *BMP280_tmpl = R"(
<div  class="card">
    <form action="/peri" method="post" class="column">
        <div class="row pos-r mb-40">
            <input type="text" name="id" class="pos-a ptitle" value="%s" readonly>
            <input type="checkbox" name="enabled" %s  class="pos-a" style="right: 30px;">
            <button type="submit" class="sv-btn brdr pos-a"><i class="fa-solid fa-floppy-disk"></i></button>
        </div>
        <table>
            <tr><td>Interface</td><td>I2C</td></tr>
            <tr><td>SDA GPIO</td><td><input type="number" name="SDA" value="%d"></td></tr>
            <tr><td>SCL GPIO</td><td><input type="number" name="SCL" value="%d"></td></tr>
            <tr><td>I2C Address (hex)</td><td><input type="text" name="I2C_ADDRESS" value="%X"></td></tr>
        </table>  
    </form>
</div>  
)"; 
class BMP280: public Peripheral{

    Adafruit_BMP280 bmp;   

    public: 
        BMP280(Preferences *prefs, int seq=1):Peripheral(prefs,seq){
            sprintf(name, "BMP280_%d", seq);

            conf["SDA"] = 4;
            conf["SCL"] = 16;
            char buf[10];
            sprintf(buf, "%d", BMP280_ADDRESS);
            conf["I2C_ADDRESS"] = buf; 
            // conf["I2C_ADDRESS"] = BMP280_ADDRESS; //BMP280_ADDRESS_ALT

            configure();
            
        }
        char * confpg(){
            char *fr = (char *) malloc(4096);
            sprintf(fr, BMP280_tmpl, name, enabled?"checked":"", sda, scl, i2caddr);
            return fr;
        }
        void init(JsonDocument *jconf) {
            Peripheral::init(jconf);
            
            sda = (uint8_t)conf["SDA"];
            scl = (uint8_t)conf["SCL"];
            i2caddr = (uint8_t)std::stoi((const char*)conf["I2C_ADDRESS"], nullptr, 16);

            if(!enabled) {
                inited = false;
                return;
            }
            
            Wire.begin(sda, scl); //SDA=GPIO4  SCL=GPIO16 

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