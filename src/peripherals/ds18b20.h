#ifndef RPZ_DS18B20_H_
#define RPZ_DS18B20_H_

#include "peripheral.h"

#include <Arduino.h>
#include <DS18B20.h>

namespace rpz{


const char *DS18B20_tmpl = R"(
<div  class="card">
    <div class="row pos-r"><h3>%s</h3><input type="checkbox" name="%s" %s  class="pos-a" style="right: 0px;"></div>
    <table>
        <tr><td>DS18B20 1</td><td class="fx">GPIO33</td></tr>
        <tr><td>DS18B20 2</td><td class="fx">GPI014</td></tr>
    </table>  
</div>  
)";     

const char *DS18B20_MSG  = R"({"temperature01":%f, "temperature02":%f})";       

class DS18B20: public Peripheral{
  public: 
    DS18B20(Preferences *prefs, int seq=1):Peripheral(prefs,seq), ds1(33), ds2(14){
        sprintf(name, "DS18B20_%d", seq);

        String pname = name; pname+="_I2CADDR";
    }
    char * confpg(){
        char *fr = (char *) malloc(4096);
        sprintf(fr, DS18B20_tmpl, name, name, enabled?"checked":"");
        return fr;
    }
    
    void init(char *_conf){

    }
    
    char * read(){
		loat temp1 = ds1.getTempC();
        delay(500); 
        float temp2 = ds1.getTempC();
        delay(500); 
        if(!isnan(temp1) && !isnan(temp2)){
            sprintf(data, DS18B20_MSG, temp1, temp2);
            return data;    
        }else   
            Serial.println("Failed to read temperature");
        return nullptr;
    }

    private:
    DS18B20 ds1;  //channel-1-DS18b20
    DS18B20 ds2;  //channel-2-DS18b20
};

}

#endif