# -*- coding: utf-8 -*-

#  Project 2 Coin Change
#  CS325.400, Group 46
#  Brouillette, David
#  Fitzwater, James
#  Stallkamp, James


from sys import argv


def changegreedy(denominations, value):

    minChange = 0
    change = 0
    amts = [0] * len(denominations)
    i = len(denominations) - 1

    for i in xrange(len(denominations) - 1, -1, -1):
        if (denominations[i] <= value):
            amts[i] += 1
            minChange += 1
            value -= denominations[i]
        else:
            i -= 1

    return amts, minChange

def changeslow(denominations, value, total):
    minCoins = [0] * len(denominations)

    if value in denominations:
        min[denominations.index(value)] += 1
        return minCoins, sum(minCoins)

    minCoins[0] = value
    for i in [coin for coin in denominations if coin <= value]:
        subQty, dummy = changeslow(denominations, value - i, total)
        subQty[denominations.index(i)] += 1
        if sum(minCoins) > sum(subQty):
            minCoins = subQty
    return sum(minCoins), minCoins

def changedp(denomination, change):
    qty = [0] * len(denomination + 1)
    minCoins = [0] * (change + 1)
    usedCoins = [0] * (change + 1)

    for subChange in range(change + 1):
        finalCoins = subChange

        if subChange == 0:
            lastVal = 0
        else:
            lastVal = 1

        for coin in denomination:
            if coin <= subChange:
                if 1 + minCoins[subChange - coin] < finalCoins:
                    finalCoins = 1 + minCoins[subChange - coin]
                    lastVal = coin

        minCoins[subChange] = finalCoins
        usedCoins[subChange] = lastVal

    coin = change
    while coin > 0:
        coinsUsed = usedCoins[coin]
        idx = denomination.index(coinsUsed)
        qty[idx] = int(qty[idx]) + 1
        coin -= coinsUsed

    return qty, minCoins[change]

# set file's names
inname = argv[1]
oname = inname.split('.')[0]
oname += 'change.txt'

# open files
inputfile = open(inname, 'r')
ofile = open(oname, 'w')


### Get problems
problems = []

while 1:
    try:
        coins = inputfile.readline()
        if coins:
            denomination = [int(num) for num in coins.replace('[', '')
                .replace(']', '')
                .replace(' ', '')
                .split(',') if num not in '\n']
        else:
            break
        amt = inputfile.readline()
        if amt:
            change = amt.replace('\n', '')
            change = int(change)
        else:
            break
    except Exception:
        break

    problems.append((denomination, change))


for problem in problems:
    qty, minCoins = changeslow(problem[0], problem[1], 1)
    ofile.write( "\nAlgorithm changeslow:\n" )
    ofile.write('{}\n'.format(problem[0]))
    ofile.write('{}\n'.format(qty))
    ofile.write('{}\n'.format(minCoins))

    qty, minCoins = changegreedy(problem[0], problem[1])
    ofile.write( "\nAlgorithm changegreedy:\n" )
    ofile.write('{}\n'.format(problem[0]))
    ofile.write('{}\n'.format(qty))
    ofile.write('{}\n'.format(minCoins))

    qty, minCoins = changedp(problem[0], problem[1])
    ofile.write( "\nAlgorithm changedp:\n" )
    ofile.write('{}\n'.format(qty))
    ofile.write('{}\n'.format(minCoins))

# close files
inputfile.close()
ofile.close()
