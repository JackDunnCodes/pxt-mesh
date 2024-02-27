#include "MeshRadio_NRF52_nrf1m.h"
#include <cstdint>
#include "Microbit.h"
#include "Utils.h"


using namespace mesh;

//variables necessary for the interrupt
MeshRadio_NRF52_nrf1m* MeshRadio_NRF52_nrf1m::instance = NULL;

void rxTurnaround(){
	NRF_RADIO->TASKS_RXEN=1;
	NRF_TIMER0->CC[0]+=768;
	NRF_PPI->TASKS_CHG[0].EN = 1;
	NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;//use rssi on reception
}


void txTurnaround(){
	NRF_RADIO->TASKS_TXEN=1;
	NRF_TIMER0->CC[0]+=800;
	NRF_PPI->TASKS_CHG[0].EN = 1;
	NRF_RADIO->SHORTS &= ~RADIO_SHORTS_ADDRESS_RSSISTART_Msk;//disables rssi for transmission
}

/**
 * 
 * simplified as the radio returns to a receiving state following a succesful window , meaning a crc failure doesnt require any special behaviour
*/
extern "C" void onDisabled(void){
	NRF_RADIO->EVENTS_END=0;
	NRF_RADIO->EVENTS_DISABLED=0;
	NRF_RADIO->EVENTS_READY=0;
	NRF_TIMER0->EVENTS_COMPARE[0]=0;
	bool complete = false;
	if(MeshRadio_NRF52_nrf1m::instance->isInReceiveMode()){//done recieving, ramping up to transmit
		if(NRF_RADIO->EVENTS_CRCOK){//only retransmit if crc ok
			NRF_RADIO->EVENTS_CRCOK = 0;
			if(MeshRadio_NRF52_nrf1m::instance->getTTL() == 0 && MeshRadio_NRF52_nrf1m::instance->txQueueEmpty()){//window end condition
				rxTurnaround();
				complete = true;
				//Event evt(DEVICE_ID_COE_RADIO, DEVICE_COE_RADIO_EVT_FLOOD_COMPLETE);
			}else{
				txTurnaround();
				if(MeshRadio_NRF52_nrf1m::instance->getTTL() > 0){
					MeshRadio_NRF52_nrf1m::instance->incrementCounter();
				}else{
					// MeshRadio_NRF52_nrf1m::instance->progressTx();
				}
				MeshRadio_NRF52_nrf1m::instance->switchTransceiveMode();
			}
			if(MeshRadio_NRF52_nrf1m::instance->getRole() == WINDOW_ROLE_PARTICIPANT && MeshRadio_NRF52_nrf1m::instance->checkPacketUnique()){
				MeshRadio_NRF52_nrf1m::instance->savePacket();
				Event evt(DEVICE_ID_COE_RADIO, DEVICE_COE_RADIO_EVT_DATA_READY);
			}else if(MeshRadio_NRF52_nrf1m::instance->getRole() == WINDOW_ROLE_CONTROLLER){//analysis on packet
				MeshRadio_NRF52_nrf1m::instance->analysePacket();
				//Event evt(DEVICE_ID_COE_RADIO, DEVICE_COE_RADIO_EVT_ANALYSIS_READY);
			}
		}else{
			rxTurnaround();
			//Event evt(DEVICE_ID_COE_RADIO, DEVICE_COE_RADIO_EVT_RECEPTION_FAILED);
		}
	}else{//ramping up to receive
		rxTurnaround();
		MeshRadio_NRF52_nrf1m::instance->switchTransceiveMode();
		if(MeshRadio_NRF52_nrf1m::instance->getTTL() == 0){
			complete = true;
				//Event evt(DEVICE_ID_COE_RADIO, DEVICE_COE_RADIO_EVT_FLOOD_COMPLETE);
		}
		//}
	}

	if(complete){
		MeshRadio_NRF52_nrf1m::instance->setFloodComplete();
	}
}

/**
 * provides highly precies timing for turning around the radio after receiving
 * look at changing from start stopping the timer to using the capture functionality to set the CC - this allows multiple tasks to use one free running timer
*/
void configureRadioPPI(){
	NRF_TIMER0->PRESCALER = 0;
	NRF_TIMER0->BITMODE = 3;
	NRF_TIMER0->CC[0] = 0;//turnaround for radio
	NRF_TIMER0->TASKS_START = 1;
	bitSet(NRF_PPI->CHEN,bit(COE_RADIO_NRF52_PPI_CHANNEL_START));
	bitSet(NRF_PPI->CHG[0],bit((COE_RADIO_NRF52_PPI_CHANNEL_START+1)) | bit((COE_RADIO_NRF52_PPI_CHANNEL_START+2)));
	NRF_PPI->TASKS_CHG[0].DIS = 1;

	// triggers the timer when a packet is received
	NRF_PPI->CH[COE_RADIO_NRF52_PPI_CHANNEL_START].EEP = (uint32_t)&NRF_RADIO->EVENTS_END;
	NRF_PPI->CH[COE_RADIO_NRF52_PPI_CHANNEL_START].TEP = (uint32_t)&NRF_TIMER0->TASKS_CAPTURE[0];
	NRF_PPI->FORK[COE_RADIO_NRF52_PPI_CHANNEL_START].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[7];
	// starts transmitting when the time is reached
	NRF_PPI->CH[COE_RADIO_NRF52_PPI_CHANNEL_START+1].EEP = (uint32_t)&NRF_TIMER0->EVENTS_COMPARE[0];
	NRF_PPI->CH[COE_RADIO_NRF52_PPI_CHANNEL_START+1].TEP = (uint32_t)&NRF_RADIO->TASKS_START;
	NRF_PPI->FORK[COE_RADIO_NRF52_PPI_CHANNEL_START+1].TEP = (uint32_t)&NRF_GPIOTE->TASKS_OUT[7];

	NRF_PPI->CH[COE_RADIO_NRF52_PPI_CHANNEL_START+2].EEP = (uint32_t)&NRF_TIMER0->EVENTS_COMPARE[0];
	NRF_PPI->CH[COE_RADIO_NRF52_PPI_CHANNEL_START+2].TEP = (uint32_t)&NRF_PPI->TASKS_CHG[0].DIS;
}


/**
 * a timeout for when events have failed to be radio driven by the transmitter
 * prgoresses to next packet in queue etc.
 * make sure this isnt triggered when its not needed
*/

void configureGPIO(){
	bitSet(NRF_GPIOTE->CONFIG[7], bit(0) | bit(1) | bit(9) | bit(8) | bit(16) | bit(17));
	bitSet(NRF_GPIOTE->CONFIG[6], bit(0) | bit(1) | bit(9) | bit(16) | bit(17));
}

void configureTests(){
	configureGPIO();
	NRF_GPIOTE->TASKS_OUT[7] = 1;
}

/**
 * constructor
*/
MeshRadio_NRF52_nrf1m::MeshRadio_NRF52_nrf1m(){
	this->instance = this;//pretty certain this got the irqhandler working although appears to have stopped again
}


/** checks the receivied packet is unique*/
bool MeshRadio_NRF52_nrf1m::checkPacketUnique(){
	return (lastRx->deviceID != rxBuf->deviceID || lastRx->packetID != rxBuf->packetID);
}

/**
 * calculates the ttl of the most recently received packet
*/
uint8_t MeshRadio_NRF52_nrf1m::getTTL(){
	if(rxBuf->maxCount == 0){//to mask cases where nothing has been receivied yet
		return 1;
	}
	return rxBuf->maxCount-rxBuf->counter;
}

MeshPayload* MeshRadio_NRF52_nrf1m::getRxBuf(){
	return rxBuf;
}
/**
 * saves the packet from the rxBuf to the receive queue to ensure it is not overwritten
*/
void MeshRadio_NRF52_nrf1m::savePacket(){
	if(rxQueue == NULL){
	 	rxQueue = new MeshPayload();
	 	rxQueueTail = rxQueue;
	}else{
		rxQueueTail->next = new MeshPayload();
		rxQueueTail = rxQueueTail->next;
	}
	if(lastRx == NULL){
		lastRx = new MeshPayload();
	}
	memcpy(lastRx,rxBuf,rxBuf->length+1);
	memcpy(rxQueueTail,rxBuf,rxBuf->length+1);
}

void MeshRadio_NRF52_nrf1m::incrementCounter(){
	rxBuf->counter++;
}
/**
 * returns true if there are no new packets to transmit
*/
bool MeshRadio_NRF52_nrf1m::txQueueEmpty(){
	if(txQueue->next == NULL){
		return true;
	}else{
		return false;
	}
}


/**
 * enables the radio before transmision can commence
 * 
 * @param flood whether packets marked for flooding should be flooded, disabling flooding on some devices in a dense network will have a minor effect on performance but may conserve these devices resources
*/
void MeshRadio_NRF52_nrf1m::enable(bool flood){
	configureTests();

	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
	while(!NRF_CLOCK->EVENTS_HFCLKSTARTED);//wait for clock to start
	NRF_RADIO->TXPOWER = 0x8;
	NRF_RADIO->FREQUENCY = 0;
	NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_1Mbit;
	bitSet(NRF_RADIO->MODECNF0, bit(0));//use fast ramp up
	NRF_RADIO->BASE0 = RADIO_BASE_ADDRESS;//addresses
	NRF_RADIO->PREFIX0 = 0;

	NRF_RADIO->TXADDRESS = 0;//logical transmission address
	NRF_RADIO->RXADDRESSES = 1;//recieve from address 0

	NRF_RADIO->PCNF0 = 0x00000008;// packet config
	NRF_RADIO->PCNF1 = 0x02040000 | (RADIO_MAX_PAYLOAD_SIZE+RADIO_HEADER_SIZE);

	NRF_RADIO->CRCCNF = RADIO_CRCCNF_LEN_Two;//crc
	NRF_RADIO->CRCINIT = 0xFFFF;
	NRF_RADIO->CRCPOLY = 0x11021;

	NRF_RADIO->DATAWHITEIV = 0x18;//initialising the data whitening
	
	bitSet(NRF_RADIO->INTENSET, bit(4)); //interrupt for disabled
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);
	NVIC_SetVector(RADIO_IRQn, (uint32_t)onDisabled);
	NVIC_SetPriority(RADIO_IRQn, 0);

	//shortcuts should use:   END_DISABLE
	NRF_RADIO->SHORTS = RADIO_SHORTS_END_DISABLE_Msk;

	if(floodingEnabled){
		configureRadioPPI();
		//setAutoDiameterCalculation(true);
	}else{

	}
	listen();
	enabled = true;
}

int MeshRadio_NRF52_nrf1m::send(MeshPayload* p){
	if(p == NULL){
		return DEVICE_INVALID_PARAMETER;
	}
	//if(!isFloodInProgress()){
		//setFloodInProgress(true);
		floodComplete = false;
		bitSet(NRF_RADIO->INTENCLR, bit(4));
		if(NRF_RADIO->STATE){//disables the radio if its already enabled
			NRF_RADIO->EVENTS_DISABLED = 0;
			NRF_RADIO->TASKS_DISABLE = 1;
			while(!NRF_RADIO->EVENTS_DISABLED);
			NRF_RADIO->EVENTS_DISABLED = 0;
		}
		txQueue=p;
		txTimeOut = txQueue;
		role = WINDOW_ROLE_CONTROLLER;
		NRF_RADIO->PACKETPTR = (uint32_t)txQueue;
		if(txEcho == NULL){// adds this to the analysis queue
			txEcho = txQueue;
			txEchoTail = txEcho;
		}else{
			txEchoTail->next = txQueue;
		}

		rxBuf->maxCount = 0;//used to protect from ttl issues when a new packet is sent as the rxbuf will still show a ttl of 0
		transceiveMode=RADIO_MODE_TRANSMIT;
		NRF_RADIO->SHORTS &= ~RADIO_SHORTS_ADDRESS_RSSISTART_Msk;

		NRF_RADIO->EVENTS_READY = 0;
		NRF_RADIO->TASKS_TXEN = 1;
		bitSet(NRF_RADIO->INTENSET, bit(4));
		while(!NRF_RADIO->EVENTS_READY);
		NRF_RADIO->TASKS_START = 1;
		NRF_GPIOTE->TASKS_OUT[7] = 1;
		NRF_RADIO->PACKETPTR = (uint32_t)rxBuf;

		return DEVICE_OK;
	//}
}

/**
 * disables the radio 
*/
void MeshRadio_NRF52_nrf1m::disable(){
	NRF_RADIO->TASKS_DISABLE = 1;
	enabled = false;
}

/**
 * starts the radios reception
*/
void MeshRadio_NRF52_nrf1m::listen(){
	role = WINDOW_ROLE_PARTICIPANT;
	NRF_RADIO->PACKETPTR = (uint32_t)rxBuf;
	transceiveMode=RADIO_MODE_RECEIVE;
	NRF_RADIO->SHORTS |= RADIO_SHORTS_ADDRESS_RSSISTART_Msk;
	NRF_RADIO->EVENTS_READY = 0;
	NRF_RADIO->TASKS_RXEN = 1;
	while(!NRF_RADIO->EVENTS_READY);
	NRF_RADIO->TASKS_START = 1;
}

/**
 * returns a single packet from the receive queue
*/
MeshPayload* MeshRadio_NRF52_nrf1m::recv(){
	if(!rxQueue){
		return NULL;
	}else{
		MeshPayload* temp = rxQueue;
		rxQueue = rxQueue->next;
		if(rxQueueTail == temp){
			rxQueueTail = NULL;
		}
		return temp;
	}
}

/**
 * gets the transceive mode
 * @return the transceive mode
*/
uint8_t MeshRadio_NRF52_nrf1m::getTransceiveMode(){
	return transceiveMode;
}

/**
 * checks whether the radio is in receive mode
 * @return true if in receive mode, false otherwise
*/
bool MeshRadio_NRF52_nrf1m::isInReceiveMode(){
	if(transceiveMode == RADIO_MODE_RECEIVE){
		return true;
	}else{
		return false;
	}
}


/**
 * sets the transcieve mode, whether the radio should be recieving or transmitting
 * @param transceiveMode the mode to set the transceiver to
*/
void MeshRadio_NRF52_nrf1m::setTransceiveMode(uint8_t transceiveMode){
	this->transceiveMode = transceiveMode;
}

/**
 * switches the transcieve mode
*/
void MeshRadio_NRF52_nrf1m::switchTransceiveMode(){
	if(transceiveMode == RADIO_MODE_RECEIVE){
		transceiveMode = RADIO_MODE_TRANSMIT;
	}else if(transceiveMode == RADIO_MODE_TRANSMIT){
		transceiveMode = RADIO_MODE_RECEIVE;
	}
}

/**
 * checks whether the flood has succesfully been completed
 * @return true if the flood is complete, false otherwise
*/
bool MeshRadio_NRF52_nrf1m::isFloodComplete(){
	return floodComplete;
}

/**
 * sets the flood to complete
*/
void MeshRadio_NRF52_nrf1m::setFloodComplete(){
	this->floodComplete = true;
	setRole(WINDOW_ROLE_PARTICIPANT);
	// resetTimeout();
}
	
void MeshRadio_NRF52_nrf1m::enableTimeSynchronisation(NRF_RTC_Type* rtc){
	this->rtc = rtc;
}