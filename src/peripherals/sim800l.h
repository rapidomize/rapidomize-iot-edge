#ifndef RPZ_SIM800_H_
#define RPZ_SIM800_H_


/*This code implements the use of ESP32 to interact with the SIM800L module, 
when GPIO36  or GPIO39 pin detects a low power level, send an SMS notification 
and print the corresponding trigger message in the serial port.*/

#include "peripheral.h" 

#include <SoftwareSerial.h>


namespace rpz{

SoftwareSerial sim800Serial(13, 5); // RX is GPIO13 TX is GPIO5
const char* simNumber = "1111111111";// this is the numner on SIM800L
const char* phoneNumber = "2222222222";// this is your phone numner (replace it)  



class SIM800: public Peripheral{
    public: 
    SIM800(Preferences *prefs):Peripheral(prefs){
    }
    char * confpg(){return "";}
    void init(char *conf){
        sim800Serial.begin(9600);
        delay(1000);

        pinMode(36, INPUT);
        pinMode(39, INPUT);

        // Check whether the module responds properly
        sendCommand("AT", "OK", 1000);
        sendCommand("AT+CSQ", "OK", 1000);
        sendCommand("AT+CREG?", "OK", 1000);
    }

    void *exec(char *){
        makeCall();

        if (digitalRead(36) == LOW) {
            Serial.println("INPUT01 IS TRIGGERED");
            sendSMS("INPUT01 IS TRIGGERED");
        }

        if (digitalRead(39) == LOW) {
            Serial.println("INPUT02 IS TRIGGERED");
            sendSMS("INPUT02 IS TRIGGERED");
        }

        return nullptr;
    } 

     void sendCommand(const char* command, const char* expectedResponse, unsigned long timeout) {
        sim800Serial.println(command);
        Serial.println("Sent command: " + String(command));
        unsigned long startTime = millis();
        while (millis() - startTime < timeout) {
            if (sim800Serial.available()) {
                String response = sim800Serial.readStringUntil('\n');
                Serial.println("Received response: " + response);
                if (response.indexOf(expectedResponse)!= -1) {
                    return;
                }
            }
        }
        Serial.println("Error: Command not responded as expected.");
    }

    void makeCall() {
        char callCommand[30];
        sprintf(callCommand, "ATD%s;", phoneNumber);
        sendCommand(callCommand, "OK", 5000);
    }

    void sendSMS(const char* message) {
        char cmgsCommand[30];
        sprintf(cmgsCommand, "AT+CMGS=\"%s\"", phoneNumber);
        sendCommand("AT+CMGF=1", "OK", 1000);
        sendCommand(cmgsCommand, ">", 1000);
        sim800Serial.println(message);
        delay(100);
        sim800Serial.write(0x1A);
        delay(100);
        if (sim800Serial.available()) {
            String response = sim800Serial.readStringUntil('\n');
            Serial.println(response);
            if (response.indexOf("OK")!= -1) {
                Serial.println("SMS sent successfully.");
            } else {
                Serial.println("Error sending SMS.");
            }
        }
    }

};

}


#endif