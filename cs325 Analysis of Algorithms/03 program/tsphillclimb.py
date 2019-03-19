#!/usr/bin/python


## James Fitzwater
## CS 325.400
## Project 3 - The Travelling Salesman Problem
## Solution Type: "Hill Climbing"



# Primary Resource:
#
#
# Marsland, Stephen. Machine Learning: An Algorithmic 
# Perspective, 2nd ed. Boca Raton, FL: CRC Press, 2015.
# seat.massey.ac.nz/personal/s.r.marsland/MLbook.html
#    TSP.py - Chap. 9, "Optimisation and Search." 204-209.



# Supplemental Resources:
#
#
#  Lutz, Mark. Learning Python, 5th ed. 
#  Sebastopol, CA: O'Reilly Media, 2013.
#     !/usr/bin/env python - Chap. 3, "How You Run Programs." 45, 47.
#     random: randint - Chap. 5, "Numeric Types." 157. 
#     loop syntax - Chap. 13, "while and for Loops." 388.
#     showiFiles.py - Adden. A, "Python Command-Line Arguments." 1432.
#
#
#  Gaddis, Tony. "Introduction to File Input and Output." 
#  Chap. 7.1 in Starting Out With Python, 2nd ed. 
#  Boston, MA: Addison-Wesley, 2012. 245, 247, 253.
#     file_write.py - Program 7-1. 245.
#     line_read.py - Program 7-3. 247.
#     write_numbers.py - Program 7-6. 253.
#
#
#  Morin, Pat. Open Data Structures (in Python), 0.1G Beta ed.
#  opendatastructures.org/ods-python.pdf 
#     adjacencylists.py - Chap. 12, "Graphs." 244-248.
#
#
#  Hock-Chuan, Chua. "Data Types and Dynamic Typing." Chap. 6 in 
#  Getting Started and Basics. Last modified January 2017.
#  ntu.edu.sg/home/ehchua/programming/webprogramming/Python1_Basics.html 
#
#
#  Hetland, Magnus Lie. Python Algorithms. New York: Apress, 2010.
#     referenced generally, or utilized for
#     double-checking syntax use
#
#
#  CS 325, project file "tsp-verifier.py"




import unittest, copy, math, random, re, sys, time
import numpy as np


## CS325 project file, tsp-verifier.py, lines 24-38
def read2(filename):
	f = open(filename,'r')
	line = f.readline()
	cities = []
	while len(line) > 1:
		lineparse = re.findall(r'[^,;\s]+', line)
		cities.append(int(lineparse[1]))
		cities.append(int(lineparse[2]))
		line = f.readline()
	f.close()
	return cities

## CS325 project file, tsp-verifier.py, lines 16-22
def distance(xa,ya,xb,yb):
	dx = xa-xb
	dy = ya-yb
	return int(round(math.sqrt(dx*dx + dy*dy)))


def hclimb( places ):

	## instantiate travel order
	#  ... copy wholly array[n][3] <--> [ID][x][y]

	#  ... get count of cities
	n = int( len(places) / 2 )

	o1 = [[0]*3 for oo in range(n)]
	o1[0][0] = 0
	o1[0][1] = int(places[1])
	o1[0][2] = int(places[2])

	for i in range(1,n):
		o1[i][0] = int(i)
		o1[i][1] = int(places[i*2])
		o1[i][2] = int(places[(i*2)+1])


	#  ---------------- 


	## instantiate travel distance
	d1 = 0
	#  ... sum distance for queue of cities
	for w in range(n):
		d1 = d1 + distance(o1[w][1],o1[w][2],o1[w-1][1],o1[w-1][2])
	

	#  ----------------  


	## shuffle order of travel-order no. 1's vals
	random.shuffle( o1 )

	## assign number of test cycles
	nTest = 1000


	#  ----------------  


	#  var - 1000 cycles
	for x in range(nTest):

		#  randomly select two city index refs.
		c1 = random.randint(0,n)
		c2 = random.randint(0,n)

		#  ... guarding against identical refs.
		if c1 != c2:

			#  instantiate copy of travel-order no. 1
			o2 = copy.copy(o1)

			## reassign: swap city references w/ adjacent elements
			#  ... using numpy bool-base test, "where()" 
			o2 = np.where(o2 == c1, -1, o2)
			o2 = np.where(o2 == c2, c1, o2)
			o2 = np.where(o2 == -1, c2, o2)

			## calc new travel distance
			d2 = 0

			for y in range(n):
				d2 = d2 + distance(o2[w][1],o2[w][2],o2[w-1][1],o2[w-1][2])

			## compare - if this dist bests most efficient
			if d2 < d1:
				#  ... case true; copy to 'One'-vars
				d1 = d2
				o1 = o2

	#  ---------------- 

	## return most-efficient travel-order, distance, num of cities
	return o1, d1, n


def main( iFile ):

	places = read2( iFile )

	print "Hill Climbing"
	start = time.time()
	trip,dist,ct = hclimb( places )
	end = time.time()

	timeBlurb = "Travel time: {}".format( str(end-start) )
	print timeBlurb
	distBlurb = "Travel Distance: {}".format(str( dist ))
	print distBlurb
	print "\n"

	oFile = open( iFile + '.tour', 'w' )
	oFile.write( "{}".format(str( dist )) )
	oFile.write( "\n" )
	for ww in range(ct): 
		oFile.write( "{}".format(str( trip[ww][0] )) )
		oFile.write( "\n" )
	oFile.close()


if __name__ == '__main__':

	main( sys.argv[1] )