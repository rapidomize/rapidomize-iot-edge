#ifndef RPZ_CLIENT_H_
#define RPZ_CLIENT_H_

#include <Preferences.h>
#include <PubSubClient.h> 

#include "provider.h"
#include "peripherals/peripheral.h"


namespace rpz{

#ifndef RPZ_VERSION
#define RPZ_VERSION "0.7.5-b28" 
#endif

enum rpz_ops_t{

	/* request ops */
	RPZ_NOP=0x00,						/* 0.00 */
	RPZ_READ=0x01,						/* 0.01 */
	RPZ_WRITE=0x02,						/* 0.02 */
	RPZ_UPDATE=0x03,					/* 0.03 */
	RPZ_DELETE=0x04,					/* 0.04 */
	RPZ_EXEC=0x05,						/* 0.05 */
	RPZ_PATCH=0x06,						/* 0.06 */
	RPZ_INFO=0x07,						/* 0.07 request for metadata if any*/
	RPZ_TUN=0x08,						/* 0.08 use in tunneling */
	RPZ_SES=0x09,						/* 0.09 sessions */

	/* Response status code */
	RPZ_CONTINUE=0x20,                  /* 1.00 */
	RPZ_PROCESSING=0x22,                /* 1.02 */

	RPZ_ACK=0x40,                       /* 2.00 success */
	RPZ_CREATED=0x41, 					/* 2.01 */
	RPZ_ACCEPTED=0x42,                  /* 2.02 request is accepted for processing */
	RPZ_CHANGED=0x44,					/* 2.04 delete/update operation success*/
	RPZ_NO_CONTENT=0x44,				/* 2.04 operation success*/
	RPZ_TUN_ACK=0x45,					/* 2.05 */

	RPZ_FOUND=0x62,				        /* 3.02 */
	RPZ_NOT_CHANGED=0x64,				/* 3.04 update, delete operation failed and no changed is made */

	/** Request Error */
	RPZ_BAD_REQUEST=0x80,				/* 4.00 */
	RPZ_UNAUTHORIZED=0x81,				/* 4.01 */
	RPZ_FORBIDDEN=0x83,					/* 4.03 */
	RPZ_NOT_FOUND=0x84,					/* 4.04 */
	RPZ_NOT_ACCEPTABLE=0x86,		    /* 4.06 */
	RPZ_REQUEST_TIMEOUT=0x88,		    /* 4.08 */
	RPZ_CONFLICT=0x89,			        /* 4.09 */
	RPZ_GONE=0x8A,			            /* 4.10 */
	RPZ_INVALID_CONTENT_SIZE=0x8B,		/* 4.11 */
	RPZ_PRECONDITION_FAILED=0x8C,		/* 4.12 */
	RPZ_PAYLOAD_TOO_LARGE=0x8D,	        /* 4.13 */
	RPZ_UNSUPPORTED_MEDIA_TYPE=0x8F,    /* 4.15 */
	RPZ_TOO_MANY_REQUESTS=0x9D,         /* 4.29 */

	/** Serving Error */
	RPZ_INTERNAL_ERROR=0xA0,			/* 5.00 */
	RPZ_BAD_GATEWAY=0xA2,				/* 5.02 */
	RPZ_SERVICE_UNAVAILABLE=0xA3,		/* 5.03 */
	RPZ_GATEWAY_TIMEOUT=0xA4,			/* 5.04 */
	RPZ_INVALID_VERSION=0xA5,		    /* 5.05 */
	RPZ_UNKNOWN=0xB4,		            /* 5.20 */

	RPZ_UNDEFINED=0xFF
};    

enum ALM{
    ALM_ACS712,
    ALM_OTHER
};

const int MAX_PERIPHERALS = 10;

class Device{
    public:    
        Device(){}

		void init();
		bool setup(bool setup=false);
        void update();
        
        bool circuit = false;

    private:
        bool connect();
        static void recv(const char* topic, byte* payload, unsigned int length);
        void send(const char* msg, const char* topic);

        const long interval = 10000;
        unsigned long prev = 0;
        int count = 0;
        
		String host;
		int port;
		bool hasSetup = false;
		ConProvider conprv;
		Preferences prefs;
		PubSubClient mqttClient;
		
		Peripheral *peripherals[MAX_PERIPHERALS];
		int pcnt = 0;
		//just an indicator to show device is alive.
		int indicate = 0;
};

} // namespace rpz




#endif //RPZ_CLIENT_H_
