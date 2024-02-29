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
    const RADIO_MAX_PACKET_SIZE = 260;
    let debug = false;
    // 
    /**
     * Send data shim. 
     *
     */
    //% shim=mesh::sendTextCpp
    export function shim_sendText(pkt: Buffer) {
        // TODO: Figure out how the simulator differentiates between micro:bit
        //       versions
        return;
    }
    /**
     * get last RX buffer
     *
     */
    //% shim=mesh::getLastRxBuffer
    export function shim_getLastRxBuffer(): Buffer {
        // TODO: Figure out how the simulator differentiates between micro:bit
        //       versions
        return null;
    }

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
        cache: string = "";
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

        get sliceIndices() {
            const offset = 7;
            return [offset, offset + this.data[0]];
        }

        get stringPayload() {
            const offset = 7;
            if(!this.cache) 
                for (let i = this.sliceIndices[0]; i < this.sliceIndices[1]; i++){
                    if (debug) serial.writeLine(">> " + this.cache);
                    let char: (string | number) = this.data[i]
                    if (typeof char  == 'string') {
                        char = parseInt(char);
                    }
                    if(this.data[i] != 0) this.cache += String.fromCharCode(char)
                }
            return this.cache;
        }

        set stringPayload(val: string) {
            this.cache = "";
            const offset = 7;
            this.packetLength = val.length < 248 ? val.length : 248;
            if (offset) {
                for (let i = 0; i < this.packetLength; i++){
                    if(debug) serial.writeLine("Write "+val.charCodeAt(i)+" from "+ i +" to "+(i+offset));
                    this.data[i+offset] = val.charCodeAt(i);
                }
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
        if (debug) {
            serial.writeBuffer(pkt.data);
            serial.writeLine("--");
            serial.writeNumber(pkt.packetLength);
            serial.writeLine("--");
            serial.writeString(pkt.stringPayload);
            serial.writeLine("=====");
        }
        return shim_sendText(pkt.data);
    }
    /**
     * Enable debug mode - output debug messages via serial.
     * Can't turn off. Sorry.
     */
    //% blockId=mbitmesh_debug
    //% block="Set debug mode on (msgs -> serial)"
    //% blockHidden=1
    export function enableDebug() {
        debug = true;
    }

    /**
     * Read in the last piece of text this node received
     */
    //% blockId=mbitmesh_recv
    //% block="last mesh net message"
    export function recv(): string {
        let buf = shim_getLastRxBuffer();
        if (buf != null)
            return "";
        let pkt = RadioPacket.getPacket(buf);
        if(debug)
        {
            serial.writeBuffer(pkt.data);
            serial.writeLine("--");
            serial.writeNumber(pkt.packetLength);
            serial.writeLine("--");
            serial.writeString(pkt.stringPayload);
            serial.writeLine("--");
            serial.writeString(pkt.data[pkt.sliceIndices[0]].toString());
            serial.writeNumber(pkt.sliceIndices[1]);
            serial.writeLine("=====");
        }
        return pkt.stringPayload;
    }
}
