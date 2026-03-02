#ifndef RPZ_AHTX0_H_
#define RPZ_AHTX0_H_

#include "peripheral.h" 

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h> 

/* 
    AHTX0 Temperature and Humidity Sensor, e.g. AHT10, AHT20
 */

namespace rpz{


const char *AHTx0_SEN_MSG  = R"({"temperature":%f, "humidity":%f})";    

const char *AHTx0_tmpl = R"(
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

class AHTx0: public Peripheral{

    Adafruit_AHTX0 aht;

    public: 
        AHTx0(Preferences *prefs, int seq=1):Peripheral(prefs,seq){
            sprintf(name, "AHTx0_%d", seq);

            conf["SDA"] = 4;
            conf["SCL"] = 16;
            char buf[10];
            sprintf(buf, "%d", AHTX0_I2CADDR_DEFAULT);
            conf["I2C_ADDRESS"] = buf; 
            // conf["I2C_ADDRESS"] =  AHTX0_I2CADDR_DEFAULT;
            configure();
        }
        
        char * confpg(){
            char *fr = (char *) malloc(4096);
            sprintf(fr, AHTx0_tmpl, name, enabled?"checked":"", sda, scl, i2caddr);
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