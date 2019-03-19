# -*- coding: utf-8 -*-

#  Project 1 MSS
#  CS325.400, Group 46
#  Brouillette, David
#  Fitzwater, James
#  Stallkamp, James


#  execute with math, random, & time libs
import math, random, time

#  define (01) enumerationAlgo
def enumerationAlgo(a):
	max = 0
	sumSoFar = 0
	start = 0
	end = 0

	for i in range(len(a)):
		for j in range(i, len(a)):
			for k in range(i, (j + 1)):
				sumSoFar = sumSoFar + a[k]
			if sumSoFar > max:
				max = sumSoFar
				start = i
				end = k
			sumSoFar = 0
	return max, start, end



# define (02) “betterEnumerationAlgo”
def betterEnumerationAlgo(a):
	max = 0
	start = 0
	end = 0

	for i in range(len(a)):
		sumSoFar = 0
		for j in range(i, len(a)):
			sumSoFar = sumSoFar + a[j]
			if sumSoFar > max:
				max = sumSoFar
				start = i
				end = j
		sumSoFar = 0
	return max, start, end



#  define helper function, “getMaxCrossingSubarray,” for (03) “divideConquerAlgo”
def getMaxCrossingSubarray(array, low, mid, high):
	leftSum = -float("inf")
	sum = 0
	maxLeft = None

	for i in range(mid, low-1, -1):
		sum += array[i]
		if sum > leftSum:
			leftSum = sum
			maxLeft = i
	rightSum = -float("inf")
	sum = 0
	maxRight = None

	for j in range(mid+1, high+1):
		sum += array[j]
		if sum > rightSum:
			rightSum = sum
			maxRight = j
	return leftSum+rightSum, maxLeft, maxRight




#  define (03) “divideConquerAlgo”
#
def divideConquerAlgo(array, low, high):
	if high == low:
#  Base Case of 1 element array
		return(array[low], low, high)
	else:
		mid = int(math.floor((low+high)/2))

		#  return tuple for left half
		leftSum, leftLow, leftHigh = divideConquerAlgo(array, low, mid)

		#  return tuple for right half
		rightSum, rightLow, rightHigh = divideConquerAlgo(array, mid+1, high)

		#  return tuple for across middle subarray
		crossSum, crossLow, crossHigh = getMaxCrossingSubarray(array, low, mid, high)

		#  subarray on left has greatest sum
		if leftSum >= rightSum and leftSum >= crossSum:
			return leftSum, leftLow, leftHigh

		#  subarray on right has greatest sum
		elif rightSum >= leftSum and rightSum >= crossSum:
			return rightSum, rightLow, rightHigh

		#  subarray on across middle has greatest sum
		else:
			return crossSum, crossLow, crossHigh




#  define (04) “linearAlgo”
def linearAlgo(a):
	maxHere = 0
	maxSoFar = 0
	sum = 0
	start = 0
	end = 0
	startFinal = 0
	endFinal = 0

	for x in range(len(a)):
		if a[x] > maxHere + a[x]:
			maxHere = a[x]
			start = x
		else:
			maxHere = maxHere + a[x]
			end = x
		if maxSoFar < maxHere:
			maxSoFar = maxHere
			startFinal = start
			endFinal = end
	return maxSoFar, startFinal, endFinal


#  open - assign “infile” to local file “MSS_Problems.txt”
#  & instantiate "ofile" to new file "Results.txt;"
#  pass 'w' flag as "write" signal, 'r' as "read;"

#infile = open('MSS_Problems.txt', 'r')
ofile = open('MSS_Results.txt', 'w')

arrayNameOutput = "Array: {}"

problems = []

with open('MSS_Problems.txt') as f:
	for line in f:
		problems.append([int(num) for num in line.split(' ') if num not in '\n'])

for problem in problems:	
	ofile.write(arrayNameOutput.format(problem))
	ofile.write("\n")

	ofile.write( "\nalgorithm 1: Enumeration\n" )
	outputResult = enumerationAlgo( problem )
	resultFormatted = "Results: {}".format( str(outputResult) )
	ofile.write( resultFormatted )

	ofile.write( "\nalgorithm 2: Better Enumeration\n" )
	outputResult = betterEnumerationAlgo( problem )
	resultFormatted = "Results: {}".format( str(outputResult) )
	ofile.write( resultFormatted )

	ofile.write( "\nalgorithm 3: Divide & Conquer\n" )
	outputResult = divideConquerAlgo( problem, 0, len(problem) - 1 )
	resultFormatted = "Results: {}".format( str(outputResult) )
	ofile.write( resultFormatted )

	ofile.write( "\nalgorithm 4: Linear\n" )
	outputResult = linearAlgo( problem )
	resultFormatted = "Results: {}".format( str(outputResult) )
	ofile.write( resultFormatted )
	ofile.write("\n\n")

		#  input/output complete: close files
#infile.close()
ofile.close()


#  execute main

