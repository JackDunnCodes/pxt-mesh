/**
 * C++ functions as part of the extension.
 * 
 * A function defined here will run on the micro:bit, there is also a "shim"
 * in the main.ts file that will be run in the simulator.
 * Info about shims: https://makecode.com/simshim
 */
#include "pxt.h"
#include "MeshRadio_NRF52_nrf1m.h"
// #include "MicroBit.h"

using namespace pxt;

namespace mesh {
    #define V1_MULTIPLIER 2
    #define V2_MULTIPLIER 3
    MeshRadio_NRF52_nrf1m* radio;
    int pid = 0;
    /**
     * Turn on the radio, and get ready for the network. You'll need to
     * do this before you can send or receive anything
     */
    //%
    void initRadioCpp() {
        radio = new MeshRadio_NRF52_nrf1m();
        radio->enable();
    }

    /**
     * Send some text
     */
    //%
    void sendTextCpp(Buffer pkt) {
        MeshPayload *p;
        memcpy(p,pkt,sizeof(MeshPayload));
        p->packetID = pid++;
        p->counter = 0;
        p->maxCount = 3;
	    // memcpy((void*)p->payload,sendString,3);
        radio->send(p);
    }

    /**
     * Get last received buffer
     */
    //%
    Buffer getLastRxBuffer() {
        mesh::MeshPayload *payload;
        memcpy(payload,radio->recv(),sizeof(MeshPayload));
        payload->payload = (uint8_t) "This is a test";
        return mkBuffer(payload,255);
    }

    /**
     * This function uses the C preprocessor to compile different code
     * depending on the version of the micro:bit.
     */
    //%
    int banana_multiplier(int bananas) {
    #if MICROBIT_CODAL
        // On micro:bit V2 the multiplier is 2
        return bananas * V2_MULTIPLIER;
    #else
        // On micro:bit V1 the multiplier is 1
        return bananas * V1_MULTIPLIER;
    #endif
    }

    /**
     * This function uses the C preprocessor to throw an error if this function
     * is run on a micro:bit V1.
     */
    //%
    int bananas_v2_only(int bananas) {
    #if MICROBIT_CODAL
        return bananas;
    #else
        target_panic(PANIC_VARIANT_NOT_SUPPORTED);
    #endif
    }
}
