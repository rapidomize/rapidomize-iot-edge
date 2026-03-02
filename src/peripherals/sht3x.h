#ifndef RPZ_SHT3X_H_
#define RPZ_SHT3X_H_

#include "peripheral.h" 

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>
/* 
    SHT3x Temperature and Humidity Sensor
 */

namespace rpz{


const char *Sht3x_tmpl = R"(
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

const char *SHT3X_SEN_MSG  = R"({"temperature":%f, "humidity":%f})"; 

class Sht3x: public Peripheral{

    Adafruit_SHT31 sht3x = Adafruit_SHT31();

    public: 
    Sht3x(Preferences *prefs, int seq=1):Peripheral(prefs,seq){
        sprintf(name, "SHT3x_%d", seq);

        //default
        conf["SDA"] = 4;
        conf["SCL"] = 16;
        char buf[10];
        sprintf(buf, "%d", SHT31_DEFAULT_ADDR); //default 0x44 - Set to 0x45 for alternate i2c addr
        conf["I2C_ADDRESS"] = buf; 
        configure();
    }

    char * confpg(){
        char *fr = (char *) malloc(4096);
        sprintf(fr, Sht3x_tmpl, name, enabled?"checked":"", sda, scl, i2caddr);
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