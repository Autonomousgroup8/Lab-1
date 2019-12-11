""" Class to represent markers
"""

import re
import time
import math
import numpy as np

# set up a regular expression to search for marker data in the stream
# The structure of the marker data is as the following example
# Marker 2
# (-8.035617, 2.949460, 171.360166)
# [(0.005031, -0.387423, 0.921888); 
# (-0.999962, -0.008453, 0.001904); 
# (0.007055, -0.921863,
# -0.387451)
# ]
# 1.000000
MARKER_REGEX = re.compile(rb'Marker\s+(?P<markernumber>[0-9])+\s+\((?P<position>.+)\)\s+\[(?P<orientation>.+)\]\s+(?P<confidence>[0-1]\.[0-9]+)')


class MarkerCollection(object):
    '''MarkerCollection keeps track of a collection of visible markers '''

    def __init__(self):
        self.markerindex = dict()
        self.visiblemarkers = dict()

    def add_marker(self, markername, number):
        '''Add a marker to the collection, associate its name 'markername' with the marker number 'number' '''
        self.markerindex[markername] = number

    def update_marker(self, marker):
        '''Update the information of a marker in the collection.'''
        self.visiblemarkers[marker.number] = marker

    def get_marker(self, markername):
        '''Get information about a marker by its name.'''
        # check if we know about this marker
        if not markername in self.markerindex:
            return None
        # check if we have an observation about this marker in the collection
        if not self.markerindex[markername] in self.visiblemarkers:
            return None
        # return the marker
        return self.visiblemarkers[self.markerindex[markername]]

    def clean_outdated_markers(self, seconds):
        '''Remove observations older than 'seconds' seconds from the collection. '''
        current_time = time.time()
        self.visiblemarkers = dict((num, marker) for num, marker in self.visiblemarkers.items() if current_time - marker.timestamp < seconds)

class Marker(object):
    """ Class to represent a detected marker.
    """

    def __init__(self, binary_string):
        '''Create a new marker from its string representation and set its time stamp to now.'''
        # find the marker in the string using the regular expression
        match_object = re.search(MARKER_REGEX, binary_string)
        # extract the marker number
        self.number = int(match_object.group('markernumber'))
        # extract the marker position
        self.position = np.array([float(x) for x in match_object.group('position').split(b',')])
        # extract the orientation matrix
        columns = match_object.group('orientation').split(b';')
        stripped = [col.strip(b'() ') for col in columns]
        colvectors = [[float(x) for x in col.split(b',')] for col in stripped]
        self.orientation = np.array(colvectors).transpose()
        # extact the marker confidence level
        self.confidence = float(match_object.group('confidence'))
        # set the time stamp of detection to the current time
        self.timestamp = time.time()

    def extended_matrix_m2c(self):
        '''Return the 4x4 matrix representing the rotation and translation that transforms a vector in marker coordinates to a vector in camera coordinates. It is a matrix [D, p; 0, 1] where submatrix D is the orientation matrix and column vector p is the position'''
        return np.block([
            [self.orientation, np.reshape(self.position, (3, 1))],
            [np.zeros((1, 3)), np.ones((1, 1))]
            ])

    def extended_matrix_c2m(self):
        '''Return the 4x4 matrix representing the rotation and translation that transforms a vector in camera coordinates to a vector in marker coordinates.'''
        # the inverse transform matrix of a matrix [D, p; 0, 1] is: [ D^T  -D^T p ;  0 1]
        inv_orientation = self.orientation.transpose()
        inv_position = np.negative(inv_orientation.dot(self.position))
        return np.block([
            [inv_orientation, np.reshape(inv_position, (3, 1))],
            [np.zeros((1, 3)), np.ones((1, 1))]
            ])

    def extended_matrix_m2m(self, marker):
        '''Return the 4x4 matrix representing the rotation and translation that transforms a vector in this markers coordinates to a vector in 'marker's coordinates.'''
        return np.dot(marker.extended_matrix_c2m(), self.extended_matrix_m2c())
        

    def relative_position(self, marker):
        '''Return the relative position of this marker to 'marker'  '''
        trans = self.extended_matrix_m2m(marker)
        return trans[0:3, 3]

    def relative_angle(self, marker):
        '''Return the relative angle of this marker to 'marker'. Note that this method assumes that the z axis of both markers are oriented in the same direction. This is typically the case when they are both lying flat. '''
        trans = self.extended_matrix_m2m(marker)
        return math.atan2(trans[1, 0], trans[0, 0])
