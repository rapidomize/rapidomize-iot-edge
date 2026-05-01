#ifndef RPZ_SWITCH_H_
#define RPZ_SWITCH_H_


#include "peripheral.h" 
#include <FunctionalInterrupt.h>

namespace rpz{

const char *DIN_tmpl = R"(
<div  class="card">
  <form action="/peri" method="post" class="column">
      <div class="row pos-r mb-40">
          <input type="text" name="id" class="pos-a ptitle" value="%s" readonly>
          <input type="checkbox" name="enabled" %s  class="pos-a" style="right: 30px;">
          <button type="submit" class="sv-btn brdr pos-a">&#x1F4BE;</button>
      </div>
      <table>
          <tr><td>GPIO</td><td><input type="number" name="GPIO" value="%d"></td></tr>
          <tr><td>Function</td><td>
              <select name="func">
                  %s
              </select></td>
          </tr>
           <tr><td>Mode</td><td>
           <select name="mode">
              %s
          </select></td>
          </tr>
      </table>    
  </form>
</div>  
)";  

const char *FUNC[]={
  "<option value=\"5\" %s>INPUT_PULLUP</option>",
  "<option value=\"9\" %s>INPUT_PULLDOWN</option>"
};

const int FUN_V[] ={INPUT_PULLUP, INPUT_PULLDOWN};

const char *MODE[5]={
  "<option value=\"1\" %s>RISING</option>",
  "<option value=\"2\" %s>FALLING</option>",
  "<option value=\"3\" %s>CHANGE</option>",
  "<option value=\"4\" %s>ONLOW</option>",
  "<option value=\"5\" %s>ONHIGH</option>",
};

const char *DIN_MSG  = R"("%s":%d)";


const unsigned long  DEBOUNCE_DELAY = 200L;  // in milliseconds  

class DIN: public Peripheral{
    uint8_t gpio;
    uint8_t func = INPUT_PULLUP;
    uint8_t mode = CHANGE;
    volatile bool triggered = false;
    // For debouncing
    unsigned long lsttime = 0;

    public: 
    DIN(Preferences *prefs, ConProvider *conprv, int seq=1):Peripheral(prefs, conprv, seq){
      sprintf(name, "DIN_%d", seq);

      //some defaults - //https://rapidomize.com/docs/solutions/iot/device/rpz-d2x2t2ux-we/ 
      conf["GPIO"] = seq == 1? 36 : 39;
      conf["func"] = INPUT_PULLUP;
      conf["mode"] = CHANGE; //RISING, FALLING, CHANGE, ONLOW, ONHIGH;
      configure();
    }

    char * confpg(){
        char *fr = (char *) malloc(TMPL_SIZE);
        String sfunc;
        for(int i=0; i < 2; i++){
          sprintf(fr, FUNC[i], (func == FUN_V[i])?"selected":"");
          sfunc += fr;
        }

        String smode;
        for(int i=0; i < 5; i++){
          sprintf(fr, MODE[i], (mode == i + 1)?"selected":"");
          smode += fr;
        }
        
        sprintf(fr, DIN_tmpl, name, enabled?"checked":"", gpio, sfunc.c_str(), smode.c_str());
        return fr;
    }

    bool init(JsonDocument *jconf) {
      Peripheral::init(jconf);
      
      auto _gpio = (uint8_t)conf["GPIO"];
      func = (uint8_t)conf["func"];
      mode = (uint8_t)conf["mode"]; 
      if(!func) func = INPUT_PULLUP;
      if(!mode) mode = CHANGE;

      //check if already setup
      if(inited) detachInterrupt(digitalPinToInterrupt(gpio));
      gpio = _gpio;

      if(!enabled) {
          inited = false;
          return false;
      }

      isr = true;
      pinMode(gpio, func);//INPUT_PULLDOWN

      //N.B. debounce in the isr in a memeber funtion err - dangerous relocation: l32r: literal placed after use  
      // attachInterrupt(digitalPinToInterrupt(gpio), std::bind(&DIN::handleInterrupt, this), FALLING);
      attachInterrupt(digitalPinToInterrupt(gpio), [this]() IRAM_ATTR {
        unsigned long now = millis();
        if (now - lsttime > DEBOUNCE_DELAY) {
          triggered = true;
          lsttime = now;
          Serial.println("isr");
        }
      }, mode);

      inited = true;
      conprv->log(PSTR("%s initialized with gpio: %d\n"), name, gpio);
      return true;
    }
    
    char * read(){
      if(!inited) return nullptr;

      if(triggered) {
          //toggle for next event
          triggered = false;
          sprintf(data, DIN_MSG, name, 1);
          return data;
      }
      return nullptr;
		}
};


}

#endif