#ifndef COE_RADIO_H
#define COE_RADIO_H

namespace mesh
{
    class MeshRadio;
    struct MeshPayload;
}

#include <cstdint>
#include <cstddef>
#include "MicroBit.h"
// #include "MeshRadioAccess.h"

//device ID
#define DEVICE_ID_COE_RADIO 50

//device event IDs
#define DEVICE_COE_RADIO_EVT_RECEPTION_SUCCESS 31
#define DEVICE_COE_RADIO_EVT_RECEPTION_FAILED 32

#define DEVICE_COE_RADIO_EVT_FLOOD_STARTED 10
#define DEVICE_COE_RADIO_EVT_FLOOD_COMPLETE 1
#define DEVICE_COE_RADIO_EVT_FLOOD_COMPLETE_RX 11
#define DEVICE_COE_RADIO_EVT_FLOOD_COMPLETE_TX 12
#define DEVICE_COE_RADIO_EVT_FLOOD_FAILED

#define DEVICE_COE_RADIO_EVT_WINDOW_COMPLETE 2
#define DEVICE_COE_RADIO_EVT_WINDOW_COMPLETE_RX 21
#define DEVICE_COE_RADIO_EVT_WINDOW_COMPLETE_TX 22
#define DEVICE_COE_RADIO_EVT_WINDOW_COMPLETE_TX_PACKETS_LEFT 23

#define DEVICE_COE_RADIO_EVT_DATA_READY 2
#define DEVICE_COE_RADIO_EVT_ANALYSIS_READY 4

// device payload numbers
#define DEVICE_COE_RADIO_PAYLOAD_No_EVENT 1
#define DEVICE_COE_RADIO_PAYLOAD_No_DATAGRAM 2
#define DEVICE_COE_RADIO_PAYLOAD_No_SCHEDULER 5

//packet flag numbers
#define COE_RADIO_FLAG_FINAL_PACKET 0b00000001
#define COE_RADIO_FLAG_SCHEDULER_CONTROLLER  0b00000010
#define COE_RADIO_FLAG_SCHEDULER_PARTICIPANT 0b00000000
#define COE_RADIO_FLAG_CROP_FLOOD 0b000001000


//general config
#define RADIO_BASE_ADDRESS             	0x75626974
#define RADIO_MAX_PAYLOAD_SIZE          248
#define RADIO_HEADER_SIZE 				6

#define RADIO_MODE_TRANSMIT 0
#define RADIO_MODE_RECEIVE 1

#define WINDOW_ROLE_CONTROLLER false
#define WINDOW_ROLE_PARTICIPANT true

//flood config
#define FLOOD_NEVER false//never floods, reduces power consumption
#define FLOOD_AUTO true //floods all packets marked for flooding

class CoeProtocol;

namespace mesh{

	struct MeshPayload{
		volatile uint8_t length;
		volatile uint8_t deviceID;//needs a longer int
		volatile uint8_t packetID;//used alongside deviceID to allow different packets to be detected
		volatile uint8_t counter = 0;
		volatile uint8_t maxCount = 0;
		volatile uint8_t flags;
		volatile uint8_t protocol;
		volatile uint8_t payload[RADIO_MAX_PAYLOAD_SIZE];

		MeshPayload* next;
		int rssi;
		int receptionCount;
	};

	struct DeviceInfo{//rename
		uint8_t deviceID;
		uint32_t timeReceived;
		uint8_t distance;
		DeviceInfo* next;
	};

	class MeshRadio: CodalComponent {
		public://protected:
			bool enabled = false;
			uint8_t windowLengthTime = 0;
			uint8_t windowLengthPackets = 0;
			bool role;
			bool autoDiameterCalculation = true;
			uint8_t networkDiameter = 3;//default diameter
			bool floodInProgress = false;

			uint8_t retransmissionAttemptCap = 0;//the number of retransmissions to attempt before moving onto the next packet in the txQueue when 0 echos are received
			uint8_t retransmissionAttempts = 0;

			// uses 2 queues to improve safety( no need to repurpose the same queue)
			MeshPayload* txQueue = NULL;// the packets to be transmitted
			MeshPayload* rxQueue = NULL;// the packets received by the radio
			MeshPayload* rxQueueTail = NULL;
			MeshPayload* rxBuf = new MeshPayload();//for receiving packets
			MeshPayload* lastRx = NULL;// the last packet received, to allow for comparison with incoming packets
			MeshPayload* txEcho = NULL; // for receptions of the transmitted packet for analysis 
			MeshPayload* txEchoTail = NULL;
			MeshPayload* txTimeOut = NULL;

			CoeProtocol* protocols[8];
			uint8_t numProtocols = 0;

			DeviceInfo* devices; 

		//public:

			/**
			 * enables the radio and starts receiving
			 * @param flood whether to forward received packets marked for flooding - disabling currently has no effect
			*/
			virtual void enable(bool flood = FLOOD_AUTO) = 0;

			/**
			 * sends the packets
			 * prior to sending, it is checked whether a flood is in progress (this could be the device sending or receiving) and if a flood is in progress, no transmission is commenced. This does not offer absolute protection, for that a controlling scheduler is needed.
			 * @return DEVICE_OK on success, or DEVICE_INVALID parameters if p is invalid
			*/
			virtual int send(MeshPayload* p) = 0;

			/**
			 * disables the radio
			*/
			virtual void disable() = 0;

			/**
			 * receives a packet from the receive queue
			*/
			virtual MeshPayload* recv() = 0;

			/**
			 * stores a receieved packet
			*/
			virtual void savePacket() = 0;

			virtual uint8_t getTransceiveMode() = 0;
			virtual bool isInReceiveMode() = 0;
			virtual void setTransceiveMode(uint8_t transceiveMode) = 0;
			virtual bool isFloodComplete() = 0;
			virtual void setFloodComplete() = 0;

			void analysePacket();

			bool isFloodInProgress();
			void setFloodInProgress(bool floodInPrgress);

			void setRole(bool role);
			bool getRole();

			uint8_t getWidth();

			void registerProtocol(CoeProtocol* protocol);
	};
}
#endif