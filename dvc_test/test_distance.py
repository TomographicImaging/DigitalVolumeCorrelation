#   Copyright 2018 Oregon State University
#   Copyright 2018 United Kingdom Research and Innovation
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#   Author(s): Edoardo Pasca (UKRI-STFC)


import numpy as np
import sys
import unittest

class Point(object):
    def __init__(self, x,y,z):
        self.pos = np.array([x,y,z])
    def distance_from(self, point):
        return np.sqrt( np.sum( ( self.pos - point.pos ) ** 2 ) )
    def __str__(self):
        return 'Point {}'.format(self.pos)

class TestPoint(unittest.TestCase):
    def test_1(self):
        p1 = Point(0,0,0)
        p2 = Point(1,0,0)
        assert p1.distance_from(p2) == 1.
    def test_2(self):
        p1 = Point(1,2,2)
        p2 = Point(1,0,0)
        np.testing.assert_almost_equal(p1.distance_from(p2) , np.sqrt(8))

def sorted_indices(listToBeSorted):
    order = range(len(listToBeSorted))
    list_3 = [i for i in zip(listToBeSorted, order)]
    list_3.sort()
    # this contains the indices of the sorted list
    return [ t[1] for t in list_3 ]

if __name__ == '__main__':
    pc = np.loadtxt(sys.argv[1])
    points = []
    for i,p in enumerate(pc):
        points.append(
            Point(p[1],p[2],p[3])
        )
        print (points[-1])
    distances = []
    for i, p1 in enumerate(points):
        dist = []
        for j, p2 in enumerate(points):
            dist.append(p1.distance_from(p2))
        distances.append(dist)
    print (distances[0])

    for i,el in enumerate(distances):
        print (sorted_indices(el))
