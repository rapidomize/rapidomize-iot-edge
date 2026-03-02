#ifndef RPZ_SWITCH_H_
#define RPZ_SWITCH_H_

#include <FunctionalInterrupt.h>
/*Input triggers the Buzzer code*/
#include "peripheral.h" 

namespace rpz{

const char *Switch_tmpl = R"(
<div  class="card">
  <form action="/peri" method="post" class="column">
      <div class="row pos-r mb-40">
          <input type="text" name="id" class="pos-a ptitle" value="%s" readonly>
          <input type="checkbox" name="enabled" %s  class="pos-a" style="right: 30px;">
          <button type="submit" class="sv-btn brdr pos-a"><i class="fa-solid fa-floppy-disk"></i></button>
      </div>
      <table>
          <tr><td>GPIO</td><td>
          <input type="number" name="GPIO" value="%d">
          <!--<select name="GPIO">
              <option value="14">14</option>
              <option value="13">13</option>
          </select>-->
          </td>
      </table>    
  </form>
    
</div>  
)";  

const char *SW_MSG  = R"({"%s":%d})";

//https://rapidomize.com/docs/solutions/iot/device/rpz-d2x2t2ux-we/  
const int IN1=36; //GPIO36
const int IN2=39; //GPIO39

volatile bool triggered = false;
// For debouncing
unsigned long lsttime = 0;
const unsigned long  DEBOUNCE_DELAY = 200L;  // in milliseconds  

//N.B. debounce in the isr in a memeber funtion err - dangerous relocation: l32r: literal placed after use  
void IRAM_ATTR handleInterrupt() {
  //Serial.println("isr1");
  unsigned long now = millis();
  if (now - lsttime > DEBOUNCE_DELAY) {
    triggered = true;
    lsttime = now;
    Serial.println("isr2");
  }
}

class Switch: public Peripheral{
    uint8_t gpio;

    public: 
    Switch(Preferences *prefs, int seq=1):Peripheral(prefs,seq){
      sprintf(name, "DIN_%d", seq);

      //defaults
      conf["GPIO"] = seq == 1? 36 : 39;
      configure();
    }

    char * confpg(){
        char *fr = (char *) malloc(4096);
        sprintf(fr, Switch_tmpl, name, enabled?"checked":"", gpio);
        return fr;
    }

    void init(JsonDocument *jconf) {
      Peripheral::init(jconf);
      
      gpio = (uint8_t)conf["GPIO"];
      if(!enabled) {
          inited = false;
          return;
      }
      
      isr = true;
      pinMode(gpio, INPUT_PULLUP);
      // attachInterrupt(digitalPinToInterrupt(gpio), std::bind(&Switch::handleInterrupt, this), FALLING);
      attachInterrupt(digitalPinToInterrupt(gpio), handleInterrupt, CHANGE);//LOW, CHANGE, RISING, FALLING
      // Parse configuration if provided
      // Format: "slave_addr,de_pin,rx_pin,tx_pin,baud_rate" or use defaults
      inited = true;
      Serial.printf(PSTR("%s initialized with gpio: %d\n"), name, gpio);
    }
    
    char * read(){
      if(!inited) return nullptr;

      if(triggered) {
          //toggle for next event
          triggered = false;
          sprintf(data, SW_MSG, name, 1);
          return data;
      }
      return nullptr;
		}
};


}

#endif