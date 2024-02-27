#ifndef MeshRadio_NRF52_H
#define MeshRadio_NRF52_H

#include "MeshRadio.h"

class MeshRadio_NRF52: public mesh::MeshRadio{
    protected:
        int transmitPower;
        uint8_t band;

    public:
        void setTransmitPower(int power);
        int getTransmitPower();

        void setFrequencyBand(uint8_t band);
        uint8_t getFrequencyBand();
};

#endif