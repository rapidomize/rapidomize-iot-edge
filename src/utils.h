#ifndef RPZ_UTILS_H_
#define RPZ_UTILS_H_

#include <Arduino.h>

#define LED 2       //on board LED if any!
#define BUZZER 12


class Utils{

    public: 
        void log(const char * fmt, ...){
            va_list args;
            char buf[1024];
            va_start(args, fmt);
            vsprintf(buf, fmt, args);
            va_end(args);
            Serial.println(buf);
            if(ev)
                ev->send(buf);
        }
        static void indicate(){
            digitalWrite(LED,HIGH);
            delay(100);
            digitalWrite(LED,LOW);
        }

        static void buzzer(int count){
            for(int i=0; i< count; i++){
                digitalWrite(BUZZER, HIGH);
                delay(100); 
                digitalWrite(BUZZER, LOW) ;
                delay(200); 
            }
        }

        AsyncEventSource *ev;
};

#endif /* RPZ_UTILS_H_ */


