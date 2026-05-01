#ifndef RPZ_MODBUS_H_
#define RPZ_MODBUS_H_

#include "peripheral.h" 

#include <Wire.h>
/* 
    Modbus RTU
 */

namespace rpz{

 enum{
    // Modbus function codes for bit access
    MB_READ_COILS          = 0x01, // 0x01 Read Coils
    MB_READ_DISC           = 0x02, // 0x02 Read Discrete Inputs
    MB_READ_HLD_REG        = 0x03, // 0x03 Read Holding Registers
    MB_READ_IN_REG         = 0x04, // 0x04 Read Input Registers
    MB_WRITE_SINGLE_COIL   = 0x05, // 0x05 Write Single Coil
    MB_WRITE_SINGLE_REG    = 0x06, // 0x06 Write Single Register
    MB_WRITE_MUL_COIL      = 0x0F, // 0x0F Write Multiple Coils
    MB_WRITE_MUL_REG       = 0x10, // 0x10 Write Multiple Registers
    MB_MASK_WRITE_REG      = 0x16, // 0x16 Mask Write Register
    MB_READ_WRITE_MUL_REG  = 0x17, // 0x17 Read Write Multiple Registers
    
    MB_ERROR_FLAG = 0x84,
    // Modbus timeout [milliseconds]
    MB_RES_TIMEOUT        = 2000   // Modbus timeout [milliseconds]
};

const char *MODBUS_tmpl = R"(
<div  class="card">
    <form action="/peri" method="post" class="column">
        <div class="row pos-r mb-40">
            <input type="text" name="id" class="pos-a ptitle" value="%s" readonly>
            <input type="checkbox" name="enabled" %s  class="pos-a" style="right: 30px;">
            <button type="submit" class="sv-btn brdr pos-a">&#x1F4BE;</button>
        </div>
        <table>
            <tr><td>Interface</td><td>RS485 - Modbus RTU</td></tr>
            <tr><td>RX GPIO</td><td><input type="number" name="RX_PIN" value="%d"></td></tr>
            <tr><td>TX GPIO</td><td><input type="number" name="TX_PIN" value="%d"></td></tr>
            <tr><td>DE/RE GPIO</td><td><input type="number" name="DE_PIN" value="%d"></td></tr>
            <tr><td>Baud Rate</td><td><input type="number" name="BAUD_RATE" value="%d"></td></tr>
            <tr><td>Slave Address (hex)</td><td><input type="text" name="SLAVE_ADDR" value="%X"></td></tr>
        </table>  
    </form>
</div>  
)";

//Modbus RTU  - Master
class Modbus: public Peripheral{
        
    public: 

    Modbus(Preferences *prefs, ConProvider *conprv, int seq=1):Peripheral(prefs, conprv, seq){
        //some defaults
        conf["RX_PIN"] = 35;        // A - RX pin (GPIO35) 
        conf["TX_PIN"] = 32;        // B - TX pin (GPIO32)
        conf["DE_PIN"] = -1;        // DE/RE pin for MAX485
        conf["BAUD_RATE"] = 9600;   // Default baud rate
    }

    char * confpg(){
        char *fr = (char *) malloc(TMPL_SIZE);
        sprintf(fr, MODBUS_tmpl, name, enabled?"checked":"", rxPin, txPin, dePin, baudRate, slvaddr);
        return fr;
    }

    bool init(JsonDocument *jconf) {
        Peripheral::init(jconf);
        
        rxPin = (uint8_t) conf["RX_PIN"];          
        txPin = (uint8_t) conf["TX_PIN"];      
        dePin = (int8_t) conf["DE_PIN"];    
        baudRate = (uint32_t) conf["BAUD_RATE"];   
        slvaddr = (uint8_t)std::stoi((const char*)conf["SLAVE_ADDR"], nullptr, 16);  

        if(!enabled) {
            inited = false;
            return false;
        }
        
        //DE/RE pin is Configured
        if(dePin >= 0){
            pinMode(dePin, OUTPUT);
            digitalWrite(dePin, LOW); // Start in receive mode
        }
        
        // Initialize HardwareSerial
        hwserial = &Serial1; //new HardwareSerial(1);
        hwserial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
        delay(100); // Allow time for initialization

        conprv->log(PSTR("%s setting up Modbus RTU. Slave: 0x%02X, RX: %d, TX: %d, DE: %d, Baud: %lu\n"), name,
                     slvaddr, rxPin, txPin, dePin, baudRate);

        return true;
    }

    protected:
    HardwareSerial* hwserial;

    bool send(uint8_t function, uint16_t startAddr, uint16_t regCount) {
        uint8_t request[8];
        
        // Build request
        request[0] = slvaddr;
        request[1] = function;
        request[2] = (startAddr >> 8) & 0xFF;
        request[3] = startAddr & 0xFF;
        request[4] = (regCount >> 8) & 0xFF;
        request[5] = regCount & 0xFF;
        
        // Calculate CRC
        uint16_t crc = crc16(request, 6);
        request[6] = crc & 0xFF;
        request[7] = (crc >> 8) & 0xFF;

        //printHexMessage( request, sizeof(request) );
        
        // Set DE/RE pin HIGH for transmit
        if(dePin >= 0){
            digitalWrite(dePin, HIGH);
            delayMicroseconds(100); 
        }
        
        // Send request
        hwserial->write(request, sizeof(request));
        hwserial->flush();
        delayMicroseconds(100);
        
        // Set DE/RE pin LOW for receive
        if(dePin >= 0){
            digitalWrite(dePin, LOW);
            delayMicroseconds(100);
        }
        
        return true;
    }
    
    // Read Modbus RTU response
    int recv(uint8_t *buffer, uint8_t expectedBytes, uint32_t timeout = MB_RES_TIMEOUT) {
        uint32_t startTime = millis();
        uint8_t index = 0;
        
        while (millis() - startTime <= timeout) {
            if(hwserial->available()){
                buffer[index++] = hwserial->read();
                if (index >= expectedBytes)
                    break;
            }
            yield();
        }
        //printHexMessage(buffer, index);
        return index; // Return number of bytes read
    }
    
    // Read holding registers
    bool readHldRegs(uint16_t startAddr, uint16_t regCount, uint16_t *values) {
        // Send request
        if (!send(MB_READ_IN_REG, startAddr, regCount)) {
            return false;
        }
        
        // Expected response: slave(1) + function(1) + byteCount(1) + data(regCount*2) + CRC(2)
        uint8_t explen = 5 + regCount * 2;
        uint8_t response[64];
        
        int recvcnt = recv(response, explen);
        if (recvcnt < 5) {
             conprv->log(PSTR("Modbus response timeout or incomplete. Bytes read: %d\n"), recvcnt);
            return false;
        }
        
        // Check slave address
        if (response[0] != slvaddr) {
             conprv->log(PSTR("Wrong slave address in response. Expected: 0x%02X, Got: 0x%02X\n"), 
                            slvaddr, response[0]);
            return false;
        }
        
        // Check function code (error if MSB set)
        if (response[1] == MB_ERROR_FLAG) {
            String err;
            switch (response[2]){
                case 0x01: err ="Illegal function"; break;
                case 0x02: err ="Illegal address"; break;
                case 0x03: err ="Illegal data"; break;
                case 0x04: err ="Slave error"; break;
                default: err = "Unknown error";
            }
            conprv->log(PSTR("Modbus error: %s, code 0x%02X\n"), err.c_str(), response[1]);
            return false;
        }
        
        if (response[1] != MB_READ_IN_REG) {
             conprv->log(PSTR("Wrong function code in response. Expected: 0x%02X, Got: 0x%02X\n"), 
                            MB_READ_IN_REG, response[1]);
            return false;
        }
        
        // Check byte count
        uint8_t byteCount = response[2];
        if (byteCount != regCount * 2) {
             conprv->log(PSTR("Wrong byte count in response. Expected: %d, Got: %d\n"), 
                            regCount * 2, byteCount);
            return false;
        }
        
        // Verify CRC
        uint16_t receivedCRC = (response[recvcnt - 1] << 8) | response[recvcnt - 2];
        uint16_t calculatedCRC = crc16(response, recvcnt - 2);
        if (receivedCRC != calculatedCRC) {
             conprv->log(PSTR("CRC error. Received: 0x%04X, Calculated: 0x%04X\n"), 
                            receivedCRC, calculatedCRC);
            return false;
        }
        
        // Extract register values
        for (uint8_t i = 0; i < regCount; i++) {
            values[i] = (response[3 + i * 2] << 8) | response[4 + i * 2];
        }
        
        return true;
    }
    
    // Modbus RTU CRC16 calculation
    uint16_t crc16(uint8_t *buffer, uint8_t length) {
        uint16_t crc = 0xFFFF;
        for (uint8_t pos = 0; pos < length; pos++) {
            crc ^= (uint16_t)buffer[pos];
            for (uint8_t i = 8; i != 0; i--) {
                if ((crc & 0x0001) != 0) {
                    crc >>= 1;
                    crc ^= 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    void printHexMessage( uint8_t values[], uint8_t sz ) {
        for (uint8_t i = 0; i < sz; i++) {
            printHexByte( values[i] );
        }
        Serial.println();
    }

    void printHexByte(byte b){
        Serial.print((b >> 4) & 0xF, HEX);
        Serial.print(b & 0xF, HEX);
        Serial.print(' ');
    }
};    


};


#endif
