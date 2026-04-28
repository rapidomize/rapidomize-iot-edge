#ifndef RPZ_SHA256_H_
#define RPZ_SHA256_H_

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace rpz{
// Initial hash values (first 32 bits of the fractional parts of the square roots of the first 8 primes)
static const uint32_t H0[] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// Round constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes)
static const uint32_t K[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

class SHA256 {
private:
    
    uint32_t h[8];
    uint64_t messageLength;
    std::vector<uint8_t> buffer;
    
    // Right rotate operation
    uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }
    
    // Right shift operation
    uint32_t shr(uint32_t x, uint32_t n) {
        return x >> n;
    }
    
    // SHA-256 logical functions
    uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }
    
    uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }
    
    uint32_t sigma0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }
    
    uint32_t sigma1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }
    
    uint32_t gamma0(uint32_t x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ shr(x, 3);
    }
    
    uint32_t gamma1(uint32_t x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ shr(x, 10);
    }
    
    // Process a 512-bit chunk (64 bytes)
    void processChunk(const uint8_t chunk[64]) {
        uint32_t w[64];
        
        // Prepare message schedule (first 16 words from chunk)
        for (int i = 0; i < 16; i++) {
            w[i] = (chunk[i*4] << 24) | (chunk[i*4+1] << 16) | 
                   (chunk[i*4+2] << 8) | (chunk[i*4+3]);
        }
        
        // Extend to 64 words
        for (int i = 16; i < 64; i++) {
            w[i] = gamma1(w[i-2]) + w[i-7] + gamma0(w[i-15]) + w[i-16];
        }
        
        // Initialize working variables
        uint32_t a = h[0];
        uint32_t b = h[1];
        uint32_t c = h[2];
        uint32_t d = h[3];
        uint32_t e = h[4];
        uint32_t f = h[5];
        uint32_t g = h[6];
        uint32_t h_val = h[7];
        
        // Main compression loop
        for (int i = 0; i < 64; i++) {
            uint32_t t1 = h_val + sigma1(e) + ch(e, f, g) + K[i] + w[i];
            uint32_t t2 = sigma0(a) + maj(a, b, c);
            h_val = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        
        // Add compressed chunk to current hash value
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += h_val;
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
    SHA256() {
        reset();
    }
    
    void reset() {
        // Reset hash values
        memcpy(h, H0, sizeof(H0));
        messageLength = 0;
        buffer.clear();
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
    
    std::vector<uint8_t> digest() {
        // Process any remaining data with padding
        if (buffer.size() > 0 || messageLength % 64 != 0) {
            padAndProcess();
        }
        
        // Convert hash to byte array (big-endian)
        std::vector<uint8_t> result(32);
        for (int i = 0; i < 8; i++) {
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

    std::string sha256(const std::string& input) {
        SHA256 hasher;
        hasher.update(input);
        return hasher.digestHex();
    }
    
    String sha1(const String& input) {
        SHA256 hasher;
        hasher.update(input.c_str(), input.length());
        return String(hasher.digestHex().c_str());
    }

    String sha256(const char* input) {
        SHA256 hasher;
        hasher.update(input, strlen(input));
        return String(hasher.digestHex().c_str());
    }
};

};

#endif /* RPZ_SHA256_H_ */

