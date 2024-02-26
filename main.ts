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
    // 
    /**
     * Send data shim. 
     *
     */
    //% shim=mesh::sendText
    export function shim_sendText(length: number, sendString: string) {
        // TODO: Figure out how the simulator differentiates between micro:bit
        //       versions
        return;
    }
    /**
     * get last RX buffer
     *
     */
    //% shim=mesh::getLastRxBuffer
    export function shim_getLastRxBuffer(): string {
        // TODO: Figure out how the simulator differentiates between micro:bit
        //       versions
        return "shimmer!";
    }
    /**
     * Turn on (AKA initialise) the radio, and get ready for the network!
     * You need to run this before you can send and receive data, because
     * this tells the Micro:bit to listen and contribute to the network.
     *
     */
    //% blockId=mbitmesh_init_radio
    //% block="initialise mesh radio"
    //% shim=mesh::initRadio
    export function shim_initRadio() {
        // TODO: Figure out how the simulator differentiates between micro:bit
        //       versions
        return;
    }

    /**
     * Send some text across the mesh network.
     */
    //% blockId=mbitmesh_send
    //% block="send %str across mesh"
    export function sendData(str: string) {
        let length = str.length;
        return shim_sendText(length, str);
    }

    /**
     * Read in the last piece of text this node received
     */
    //% blockId=mbitmesh_recv
    //% block="send %str across mesh"
    export function recv(): string {
        return shim_getLastRxBuffer();
    }
}
