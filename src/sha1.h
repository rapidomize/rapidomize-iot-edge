#ifndef RPZ_SHA1_H_
#define RPZ_SHA1_H_


#include <Arduino.h>

#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace rpz{

class SHA1 {
private:
    uint32_t h[5];  // Hash values
    uint64_t messageLength;  // Total message length in bits
    std::vector<uint8_t> buffer;
    
    // Left rotate operation
    uint32_t rotl(uint32_t x, uint32_t n) {
        return (x << n) | (x >> (32 - n));
    }
    
    // SHA-1 logical functions
    uint32_t ch(uint32_t x, uint32_t y, uint32_t z, int t) {
        if (t < 20) {
            return (x & y) ^ (~x & z);
        } else if (t < 40) {
            return x ^ y ^ z;
        } else if (t < 60) {
            return (x & y) ^ (x & z) ^ (y & z);
        } else {
            return x ^ y ^ z;
        }
    }
    
    uint32_t constant(int t) {
        if (t < 20) {
            return 0x5A827999;
        } else if (t < 40) {
            return 0x6ED9EBA1;
        } else if (t < 60) {
            return 0x8F1BBCDC;
        } else {
            return 0xCA62C1D6;
        }
    }
    
    void processChunk(const uint8_t chunk[64]) {
        uint32_t w[80];
        
        // Prepare message schedule (first 16 words)
        for (int i = 0; i < 16; i++) {
            w[i] = (chunk[i*4] << 24) | (chunk[i*4+1] << 16) | 
                   (chunk[i*4+2] << 8) | (chunk[i*4+3]);
        }
        
        // Extend to 80 words
        for (int i = 16; i < 80; i++) {
            w[i] = rotl(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
        }
        
        // Initialize working variables
        uint32_t a = h[0];
        uint32_t b = h[1];
        uint32_t c = h[2];
        uint32_t d = h[3];
        uint32_t e = h[4];
        
        // Main compression loop
        for (int i = 0; i < 80; i++) {
            uint32_t temp = rotl(a, 5) + ch(b, c, d, i) + e + w[i] + constant(i);
            e = d;
            d = c;
            c = rotl(b, 30);
            b = a;
            a = temp;
        }
        
        // Add compressed chunk to current hash value
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }
    
    void padAndProcess() {
        uint64_t originalLengthBits = messageLength * 8;
        
        // Append '1' bit (0x80 byte)
        buffer.push_back(0x80);
        
        // Pad with zeros until length ≡ 56 mod 64
        while (buffer.size() % 64 != 56) {
            buffer.push_back(0x00);
        }
        
        // Append original length as 64-bit big-endian integer
        for (int i = 7; i >= 0; i--) {
            buffer.push_back((originalLengthBits >> (i * 8)) & 0xFF);
        }
        
        // Process remaining chunks
        for (size_t i = 0; i < buffer.size(); i += 64) {
            processChunk(&buffer[i]);
        }
    }
    
public:
    SHA1() {
        reset();
    }
    
    void reset() {
        // Initial hash values (from the spec)
        h[0] = 0x67452301;
        h[1] = 0xEFCDAB89;
        h[2] = 0x98BADCFE;
        h[3] = 0x10325476;
        h[4] = 0xC3D2E1F0;
        messageLength = 0;
        buffer.clear();
        buffer.reserve(128);  // Pre-allocate to avoid frequent reallocations
    }
    
    void update(const void* data, size_t length) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        
        for (size_t i = 0; i < length; i++) {
            buffer.push_back(bytes[i]);
            
            // Process chunk when buffer reaches 64 bytes
            if (buffer.size() == 64) {
                processChunk(&buffer[0]);
                messageLength += 64;
                buffer.clear();
            }
        }
    }
    
    void update(const std::string& str) {
        update(str.c_str(), str.length());
    }
    
    // For Arduino/ESP32 String type
    void update(const String& str) {
        update(str.c_str(), str.length());
    }
    
    std::vector<uint8_t> digest() {
        // Process any remaining data with padding
        if (buffer.size() > 0 || messageLength % 64 != 0) {
            padAndProcess();
        }
        
        // Convert hash to byte array (big-endian)
        std::vector<uint8_t> result(20);
        for (int i = 0; i < 5; i++) {
            result[i*4]   = (h[i] >> 24) & 0xFF;
            result[i*4+1] = (h[i] >> 16) & 0xFF;
            result[i*4+2] = (h[i] >> 8) & 0xFF;
            result[i*4+3] = h[i] & 0xFF;
        }
        
        return result;
    }
    
    std::string digestHex() {
        std::vector<uint8_t> hash = digest();
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (uint8_t byte : hash) {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }
    
    // Memory-efficient hex conversion (no stringstream overhead)
    void digestHex(char* output, size_t outputSize) {
        if (outputSize < 41) return;  // Need at least 41 chars (40 hex + null)
        
        std::vector<uint8_t> hash = digest();
        const char hexChars[] = "0123456789abcdef";
        
        for (int i = 0; i < 20; i++) {
            output[i*2] = hexChars[hash[i] >> 4];
            output[i*2+1] = hexChars[hash[i] & 0x0F];
        }
        output[40] = '\0';
    }

    std::string sha1(const std::string& input) {
        SHA1 hasher;
        hasher.update(input);
        return hasher.digestHex();
    }

    String sha1(const String& input) {
        SHA1 hasher;
        hasher.update(input);
        return String(hasher.digestHex().c_str());
    }

    String sha1(const char* data) {
        SHA1 hasher;
        hasher.update(data, strlen(data));
        return String(hasher.digestHex().c_str());
    }
};

};

#endif /* RPZ_SHA1_H_ */