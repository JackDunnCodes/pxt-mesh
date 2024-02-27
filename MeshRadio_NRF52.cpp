#include "MeshRadio_NRF52.h"
// #include "Microbit.h"

#define COE_RADIO_POWER_LEVELS 9

const uint8_t COE_RADIO_POWER_LEVEL[] = {0xD8, 0xEC, 0xF0, 0xF4, 0xF8, 0xFC, 0x00, 0x04, 0x08};

/**
 * sets the tranmsit power, same structure as the microbit power setting
*/
void MeshRadio_NRF52::setTransmitPower(int power){
    if(power < 0 || power >= COE_RADIO_POWER_LEVELS){

    }else{
        transmitPower = power;
        NRF_RADIO->TXPOWER = (uint32_t)COE_RADIO_POWER_LEVEL[power];
    }
}

/**
 * returns the tranmsit power
*/
int MeshRadio_NRF52::getTransmitPower(){
    return transmitPower;
}

/**
 * sets the frequency band, only takes effect if the radio is disabled
 * @param band the frequency band
*/
void MeshRadio_NRF52::setFrequencyBand(uint8_t band){
    if (band<=100){
        if(NRF_RADIO->FREQUENCY != (uint32_t)band && !NRF_RADIO->STATE){
            this->band = band;
            NRF_RADIO->FREQUENCY = (uint32_t)band;
        }
    }
}

uint8_t MeshRadio_NRF52::getFrequencyBand(){
    return band;
}