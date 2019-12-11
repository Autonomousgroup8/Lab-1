'''Communication with a Zigbee dongle on a serial port'''

import serial
import threading
from time import sleep

IO_PERIOD = 0.2

class Zigbee(object):
    '''Communication with a Zigbee dongle on a serial port'''

    def __init__(self, _serialport, _baud):
        self._input_buffer = b''
        self._output_buffer = b''
        self.serialport = _serialport
        self.serial = serial.Serial(self.serialport, _baud)
        if self.serial.isOpen():
            print('Opened Zigbee serial port.')
        else:
            print('Failed to open Zigbee serial port!')
        thread = threading.Thread(target= (lambda : self.IOLoop()))
        thread.start()

    def write(self, str):
        if not isinstance(str, bytes):
            print("str must be bytes in Zigbee.write.")
        self._output_buffer += str

    def read(self):
        data = self._input_buffer
        self._input_buffer = b''
        return data

    def close(self):
        self.serial.close()

    def IOLoop(self):
        while True:
            #print("I am looping")
            sleep(IO_PERIOD)
            # write data in output buffer
            self.serial.write(self._output_buffer)
            self._output_buffer = b''
            # read data in input buffer
            self._input_buffer += self.serial.read_all()

