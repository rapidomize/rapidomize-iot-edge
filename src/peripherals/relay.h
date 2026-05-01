#ifndef RPZ_RELAY_H_
#define RPZ_RELAY_H_


/*Relay output code*/
#include "peripheral.h" 

namespace rpz{

const char *Relay_tmpl = R"(
<div  class="card">
  <form action="/peri" method="post" class="column">
      <div class="row pos-r mb-40">
          <input type="text" name="id" class="pos-a ptitle" value="%s" readonly>
          <input type="checkbox" name="enabled" %s  class="pos-a" style="right: 30px;">
          <button type="submit" class="sv-btn brdr pos-a">&#x1F4BE;</button>
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

const char *RL_MSG  = R"({"%s":%d})";

//https://rapidomize.com/docs/solutions/iot/device/rpz-d2x2t2ux-we/
const int R1 = 2;  //GPIO2
const int R2 = 15; //GPI015

class Relay: public Peripheral{

   uint8_t gpio;

  public: 
    Relay(Preferences *prefs, ConProvider *conprv, int seq=1):Peripheral(prefs, conprv, seq){
      sprintf(name, "Relay_%d", seq);

      //defaults
      conf["GPIO"] = seq == 1? 2 : 15;
      configure();
    }
    char * confpg(){
        char *fr = (char *) malloc(TMPL_SIZE);
        sprintf(fr, Relay_tmpl, name, enabled?"checked":"", gpio);
        return fr;
    }
    bool init(JsonDocument *jconf) {
      Peripheral::init(jconf);
      gpio = (uint8_t)conf["GPIO"];

      if(!enabled) {
          inited = false;
          return false;
      }
      
      pinMode(gpio, OUTPUT);

      inited = true;
      conprv->log(PSTR("%s initialized.\n"), name);
      return true;
    }

    void *write(char *data){
      if(!inited) return nullptr;
      
      digitalWrite(gpio,HIGH);
      delay(100); 
      digitalWrite(gpio,LOW) ;
      delay(100); 
      return nullptr;
    }
};

}

#endif 