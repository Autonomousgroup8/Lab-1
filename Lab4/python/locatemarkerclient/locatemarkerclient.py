""" Client to connect to a locatemarkers program to receive the marker data, do some computation on that data and show how to communicate on the Zigbee connection.
"""

import socket
import re
import numpy as np
from time import sleep
from operator import add
import matplotlib as mpl
import matplotlib.pyplot as plt
from numpy.linalg import norm
from marker import Marker, MarkerCollection, MARKER_REGEX
from zigbee import Zigbee
from pathfinding import PathFinding

# forget markers after not seeing them for 10 seconds
MARKER_TIMEOUT = 10

# Create a marker collection and register known markers with their numbers.
# Set Marker to other for testing
MARKERS = MarkerCollection()
MARKERS.add_marker('origin', 2)
MARKERS.add_marker('robotA', 5)


# connect to the locatemarkers program tcp server
# IP address for the connection '127.0.0.1' means 'this computer'
TCP_IP = '127.0.0.1'
# The program listens on port 4242
TCP_PORT = 4242
# create an IP socket for communication
LMSOCKET = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# connect the socket to the server
LMSOCKET.connect((TCP_IP, TCP_PORT))
# set the socket to non-blocking, so calls to 'recv' will not block when there is no information to receive from the connection
LMSOCKET.setblocking(0)

# Create a Zigbee object for communication with the Zigbee dongle
# Make sure to set the correct COM port and baud rate!
# You can find the com port and baud rate in the xctu program.
ZIGBEE = Zigbee('COM10', 9600)

# DATA holds the unprocessed data from the connection. Its length will be limited to BUFFER_SIZE
BUFFER_SIZE = 1024
DATA = b""
MAIN_GRID = np.zeros([150, 150])
MAIN_GRID[0][0] = 1
# loop forever


def add_to_grid(rel, grid, name=0):
    x_v = rel[0]
    x_y = rel[1]
    grid[int(abs(round(x_v)))][int(abs(round(y_v)))] = name


def check_markers(coll_a=MARKERS, name='robotA'):
    origin = coll_a.get_marker('origin')
    if (not origin is None):
        rob = coll_a.get_marker(name)
        if(not rob is None):
            rel = name.relative_position(origin)
            return rel
        return 0


while True:

    # read new data (if any) from the TCP connection
    try:
        DATA += LMSOCKET.recv(BUFFER_SIZE)
    except socket.error:
        # no data is available on the socket, sleep for 100ms
        sleep(0.1)
        # try again
        continue

    # check the data against the marker regular expression to check if there is complete marker information in the received data
    MARKER_RE_MATCH = re.search(MARKER_REGEX, DATA)

    # as long as we still have more complete marker information...
    while MARKER_RE_MATCH != None:
        # find the first marker information from the received data
        MARKER = Marker(MARKER_RE_MATCH.group())
        # show that we have found something
        # print("Found marker {0}.".format(MARKER.number))
        # update information in the marker collection
        MARKERS.update_marker(MARKER)
        # remove outdated observations
        MARKERS.clean_outdated_markers(MARKER_TIMEOUT)
        # remove the processed data from the buffer
        DATA = DATA[MARKER_RE_MATCH.end():]
        # look for the next marker in the buffer
        MARKER_RE_MATCH = re.search(MARKER_REGEX, DATA)

        # send something arbitrary to the Zigbee dongle. This has no real function except to demonstrate how to send something.
        ZIGBEE.write(b'Hello')


    if len(DATA) > BUFFER_SIZE:
        # something is going wrong, flush garbage data
        DATA = ""

    # use the results to get the position and orientation of one marker (robot) relative to another marker (origin)
    tempPos = check_markers(MARKERS, 'robotA')
    add_to_grid(tempPos, MAIN_GRID, 1)
    sleep(5)
    # if both are present
    # if (not ORIGIN is None) and (not ROBOT is None):
    # compute and print the relative position
    # print('Position: ({0:.2f}, {1:.2f}, {2:.2f})'.format(P[0], P[1], P[2]))
    # compute and print the relative orientation
    # print('Angle: {0:.2f}'.format(ROBOT.relative_angle(ORIGIN)))

# close the socket (if we ever get here)
LMSOCKET.close()
