#include "MeshRadio_NRF52.h"

#define COE_RADIO_NRF52_PPI_CHANNEL_START 10
#define COE_RADIO_NRF52_GPIOTE_CHANNEL 7

namespace codal{
	class MeshRadio_NRF52_nrf1m: public MeshRadio_NRF52{
		
		public:
			bool floodingEnabled = true;
			uint8_t transceiveMode;
			bool floodComplete = false;
			static MeshRadio_NRF52_nrf1m* instance;

			NRF_RTC_Type* rtc;

			MeshRadio_NRF52_nrf1m();


			void enable(bool flood = FLOOD_AUTO);
			int send(CoePayload* p);
			void disable();
			void listen();
			CoePayload* recv();
			void incrementCounter();
			bool txQueueEmpty();
			void progressTx();
			bool txProgressed();
			void updateTimeout();
			void resetTimeout();
			uint8_t getTTL();
			CoePayload* getRxBuf();
			void savePacket();
			bool checkPacketUnique();
			uint8_t getTransceiveMode();
			bool isInReceiveMode();
			void setTransceiveMode(uint8_t transceiveMode);
			void switchTransceiveMode();
			bool isFloodComplete();
			void setFloodComplete();
			bool isDataReady();

			void enableTimeSynchronisation(NRF_RTC_Type* rtc);
	};
}