#ifndef RPZ_SIM7600_H_
#define RPZ_SIM7600_H_


/*
  The function of this code is that when GPIO36(input01) is triggered,
  SIM7600 will call the set number and send a text message
*/
#include "peripheral.h"

#include <SoftwareSerial.h>

namespace rpz{

SoftwareSerial simSerial(13, 5); // RX is GPIO13, TX is GPIO5

const int IN1 = 36;

const char phoneNumber[] = "0123456789"; // This is the  phone number your want to text(Replace it)
const char message[] = "input01 is triggered";

class SIM7600 : public Peripheral{
  public:
    SIM7600(Preferences *prefs):Peripheral(prefs){
    }
    char * confpg(){return "";}
    void init(char * conf){
      simSerial.begin(115200); // the sim7600  baud rate is 115200
      pinMode(IN1, INPUT);
    }

    void *exec(char *){
      if (digitalRead(IN1) == LOW){
        makeCall(phoneNumber);
        delay(2000);
        sendSMS(phoneNumber, message);
        delay(5000);
      }
      return nullptr;
    }

    void makeCall(const char *number){
      simSerial.println("ATD" + String(number) + ";");
      delay(5000);
      if (!checkResponse("OK")) {
        Serial.println("Failed to make call.");
      }
    }

    bool sendSMS(const char *number, const char *text){
      simSerial.println("AT");
      delay(1000);
      if (!checkResponse("OK"))
        return false;

      simSerial.println("AT+CMGF=1");
      delay(1000);
      if (!checkResponse("OK"))
        return false;

      simSerial.print("AT+CMGS=\"");
      simSerial.print(number);
      simSerial.println("\"");
      delay(1000);
      if (!checkResponse(">"))
        return false;

      simSerial.print(text);
      simSerial.write(0x1A); // ASCII code for Ctrl+Z
      delay(5000);
      return checkResponse("OK");
    }

    bool checkResponse(const char *expected){
      String response = "";
      while (simSerial.available()){
        response += char(simSerial.read());
        delay(10);
      }
      Serial.print("Received response: ");
      Serial.println(response);
      return response.indexOf(expected) != -1;
    }
  };

}

#endif