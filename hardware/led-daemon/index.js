// simple daemon to run the LED board in the fez9000.
// accepts commands via UDP in "FUDI" format from PureData
// to drive the LED board.

const { SerialPort } = require('serialport')
const dgram = require('dgram');

// open the usb serial port, depends on udev rule setting the device
// to the right name (/dev/ttyLEDS)
const port = new SerialPort({ path: '/dev/ttyLEDS', baudRate: 9600 }, function (err) {
  if (err) {
      console.log('Error: ', err.message)
      process.exit(1);      
  }
})


// make sure the port is closed because weird things happen when it
// isnt.
process.on('exit', (code) => {
    console.log("EXIT: closing serial port.");
    port.close();
});

process.on('SIGINT', function handleSigterm() {
    console.log("EXIT: closing serial port.");
    port.close();
    process.exit();
});

// cmd 1: set parameter
function sendParam(param, num)
{
    const buf = Buffer.alloc(5, 0)
    buf[0] = 250;
    buf[1] = 1; // cmd 1 beat
    buf[2] = param & 12; // not used in cmd 1
    buf[3] = num & 0xF; 
    buf[4] = 255; // stop byte
    port.write(buf);
}

// cmd 2: beat number
function sendBeatNumber(num)
{
    const buf = Buffer.alloc(5, 0)
    buf[0] = 250;
    buf[1] = 2; // cmd 1 beat
    buf[2] = num & 0xFF; // not used in cmd 1
    buf[3] = 0; 
    buf[4] = 255; // stop byte
    port.write(buf);
}



const server = dgram.createSocket('udp4');

server.on('error', (err) => {
    console.log(`server error:\n${err.stack}`);
    server.close();
});

server.on('message', (pkt, rinfo) => {
    var messages = pkt.toString().split(";");
    for (var i = 0 ; i < messages.length; i++) {
	const msg = messages[i];
	const atoms = msg.split(/\s+/);

	if (atoms[0] == 'beat') {
	    sendBeatNumber(parseInt(atoms[1]));
	}

	if (atoms[0] == 'param' & atoms.length > 2) {
	    sendParam(parseInt(atoms[1]), parseInt(atoms[2]));
	}
    }
});

server.on('listening', () => {
    const address = server.address();
    console.log(`server listening ${address.address}:${address.port}`);
});

server.bind(31337);
