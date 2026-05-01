#ifndef RPZ_I2C_H_
#define RPZ_I2C_H_

#include "peripheral.h" 

#include <Wire.h>
/* 
    Generic I2C
 */

namespace rpz{


const char *I2C_tmpl = R"(
<div  class="card">
    <form action="/peri" method="post" class="column">
        <div class="row pos-r mb-40">
            <input type="text" name="id" class="pos-a ptitle" value="%s" readonly>
            <input type="checkbox" name="enabled" %s  class="pos-a" style="right: 30px;">
            <button type="submit" class="sv-btn brdr pos-a">&#x1F4BE;</button>
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

class I2C: public Peripheral{

    public: 
        I2C(Preferences *prefs, ConProvider *conprv, int seq=1):Peripheral(prefs, conprv, seq){
             //some defaults - //https://rapidomize.com/docs/solutions/iot/device/rpz-d2x2t2ux-we/ 
             //each impls will override
            conf["SDA"] = 4;
            conf["SCL"] = 16;
            conf["I2C_ADDRESS"] = ""; 
        }
        
        char * confpg(){
            char *fr = (char *) malloc(TMPL_SIZE);
            sprintf(fr, I2C_tmpl, name, enabled?"checked":"", sda, scl, i2caddr);
            return fr;
        }

        bool init(JsonDocument *jconf) {
            Peripheral::init(jconf);
            
            sda = (uint8_t)conf["SDA"];
            scl = (uint8_t)conf["SCL"];
            i2caddr = (uint8_t)std::stoi((const char*)conf["I2C_ADDRESS"], nullptr, 16);

            if(!enabled) {
                inited = false;
                return false;
            }
            
            Wire.begin(sda, scl); //SDA=GPIO4  SCL=GPIO16 
            return true;
		}
};

}

#endif