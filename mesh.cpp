/**
 * C++ functions as part of the extension.
 * 
 * A function defined here will run on the micro:bit, there is also a "shim"
 * in the main.ts file that will be run in the simulator.
 * Info about shims: https://makecode.com/simshim
 */
#include <cstdlib>
#include "pxt.h"
#include "MeshRadio.h"
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
        MeshPayload *p = new MeshPayload;
        memset(p, 0, sizeof(MeshPayload));
        memcpy(p,pkt->data,sizeof(MeshPayload));
        p->packetID = pid++;
        p->counter = 0;
        p->maxCount = 3;
        // For some reason, the payload is simply not sent at all, unless length is at least 3.
        // TODO:FIXME I don't know why this is, and telling Tx that the packet is longer than it is seems bad.
        if(p->length < 3) {
            p->length = 3;
        }
        radio->send(p);
        free(p);
    }

    /**
     * Get last received buffer
     */
    //%
    Buffer getLastRxBuffer() {
        uint8_t buf[sizeof(MeshPayload)];
        memset(buf, 0, sizeof(buf));
        MeshPayload* payload = radio->recv();
        if(payload != NULL) {
            memcpy(buf,payload,sizeof(buf));
            // buf[7] = 'h';
            Buffer returnBuf = mkBuffer(buf, sizeof(buf));
            free(buf);
            return returnBuf;
        } else
            return NULL;
    }
    void onDataReceived(MicroBitEvent e, void *action) {
            // if (radioEnable() != DEVICE_OK) return;
            runAction1((Action)action, fromInt(e.value));
        }
    //%
    void onDataReceivedRegister(Action body) {
        // if (radioEnable() != DEVICE_OK) return;
        uBit.messageBus.listen(DEVICE_ID_COE_RADIO, DEVICE_COE_RADIO_EVT_DATA_READY, onDataReceived, body); 
    }

    //%
    

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
