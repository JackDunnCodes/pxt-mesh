/**
 * Provides access to all the banana functionality.
 * 
 * Function docstrings appear as info the in JavaScript toolbox.
 * 
 * A function defined here with a "shim" entry will run as it is here in the
 * simulator, and the C++ function will run in the micro:bit.
 * Info about shims: https://makecode.com/simshim
 * Info about defining blocks: https://makecode.com/defining-blocks
 */

// Icon unicode characters can be found at: http://fontawesome.io/icons/
//% color=#f44708 weight=100 icon="\uf542" block="mesh:bit" advanced=false
namespace mesh {
    const RADIO_MAX_PACKET_SIZE = 248 + 7;

    function truncateString(str: string, bytes: number) {
        str = str.slice(0, bytes);
        let buff = control.createBufferFromUTF8(str);

        while (buff.length > bytes) {
            str = str.substr(0, str.length - 1);
            buff = control.createBufferFromUTF8(str);
        }

        return str;
    }

    /**struct MeshPayload{
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
	}; */
    export class RadioPacket {
        public static getPacket(data: Buffer) {
            if (!data) return undefined;
            return new RadioPacket(data);
        }

        public static mkPacket() {
            const res = new RadioPacket();
            return res;
        }

        private constructor(public readonly data?: Buffer) {
            if (!data) this.data = control.createBuffer(RADIO_MAX_PACKET_SIZE + 4);
        }

        // get signal() {
        //     return this.data.getNumber(NumberFormat.Int32LE, this.data.length - 4);
        // }

        get packetLength() {
            return this.data[0];
        }
        set packetLength(length: number) {
            this.data[0] = length;
        }

        get stringPayload() {
            const offset = 7;
            return offset ? this.data.slice(offset, offset + this.packetLength).toString() : undefined;
        }

        set stringPayload(val: string) {
            const offset = 7;
            if (offset) {
                const buf = control.createBufferFromUTF8(truncateString(val, 248));
                this.packetLength = buf.length;
                this.data.write(offset, buf);
            }
        }

        // get numberPayload() {
        //     switch (this.packetType) {
        //         case PACKET_TYPE_NUMBER:
        //         case PACKET_TYPE_VALUE:
        //             return this.data.getNumber(NumberFormat.Int32LE, PACKET_PREFIX_LENGTH);
        //         case PACKET_TYPE_DOUBLE:
        //         case PACKET_TYPE_DOUBLE_VALUE:
        //             return this.data.getNumber(NumberFormat.Float64LE, PACKET_PREFIX_LENGTH);
        //     }
        //     return undefined;
        // }

        // set numberPayload(val: number) {
        //     switch (this.packetType) {
        //         case PACKET_TYPE_NUMBER:
        //         case PACKET_TYPE_VALUE:
        //             this.data.setNumber(NumberFormat.Int32LE, PACKET_PREFIX_LENGTH, val);
        //             break;
        //         case PACKET_TYPE_DOUBLE:
        //         case PACKET_TYPE_DOUBLE_VALUE:
        //             this.data.setNumber(NumberFormat.Float64LE, PACKET_PREFIX_LENGTH, val);
        //             break;
        //     }
        // }
    }
    /**
     * Turn on (AKA initialise) the radio, and get ready for the network!
     * You need to run this before you can send and receive data, because
     * this tells the Micro:bit to listen and contribute to the network.
     *
     */
    //% blockId=mbitmesh_init_radio
    //% block="initialise mesh radio"
    //% shim=mesh::initRadioCpp
    export function initRadio() {
        // TODO: Figure out how the simulator differentiates between micro:bit
        //       versions
        return;
    }

    /**
     * Send some text across the mesh network.
     */
    //% blockId=mbitmesh_send
    //% block="send %str across mesh"
    export function sendText(str: string) {
        let pkt = RadioPacket.mkPacket();
        pkt.stringPayload = str;
        return sendTextCpp(pkt.data);
    }

    /**
     * Read in the last piece of text this node received
     */
    //% blockId=mbitmesh_recv
    //% block="last mesh net message"
    export function recv(): string {
        let buf = getLastRxBuffer();
        let pkt = RadioPacket.getPacket(buf);
        return pkt.stringPayload;
    }
}
