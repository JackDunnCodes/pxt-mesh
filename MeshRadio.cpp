
#include "MeshRadio.h"
using namespace codal;

bool MeshRadio::isFloodInProgress(){
    return floodInProgress;
}
void MeshRadio::setFloodInProgress(bool floodInPrgress){
    this->floodInProgress = floodInProgress;
}

void MeshRadio::analysePacket(){
	if(txEcho->deviceID == rxBuf->deviceID && txEcho->packetID == rxBuf->packetID){
		if(rxBuf->rssi<txEchoTail->rssi){
			txEchoTail->rssi = rxBuf->rssi;
		}
		txEchoTail->receptionCount++;
	}
}


bool MeshRadio::getRole(){
    return role;
}

void MeshRadio::setRole(bool role){
    this->role = role;
}

uint8_t MeshRadio::getWidth(){
    return networkDiameter;
}

/**
 * registers a protocol, the protocols callback will be triggered whenever a packet with a matching protocol number is received
*/
void MeshRadio::registerProtocol(CoeProtocol* protocol){
    if(numProtocols<8){
        numProtocols++;
        protocols[numProtocols-1] = protocol;
    }
}
