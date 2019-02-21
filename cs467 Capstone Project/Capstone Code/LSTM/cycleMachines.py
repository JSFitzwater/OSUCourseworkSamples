from datetime import datetime, timedelta
from pandas_datareader import data
import csv
import json
import time
import math
import shutil
import subprocess
import pandas as pd
import numpy as np
import urllib.request, json
import datetime as dt
from datetime import date, timedelta
from numpy import concatenate
from matplotlib import pyplot
import matplotlib.ticker as ticker
from pandas import read_csv
from pandas import DataFrame
from pandas import concat
from core.data_processor import DataLoader
from core.model import Model
from sklearn.preprocessing import MinMaxScaler
import os
import sys
stderr = sys.stderr
sys.stderr = open(os.devnull, 'w')
import tensorflow as tf
sys.stderr = stderr
tf.logging.set_verbosity(tf.logging.FATAL)
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'


# docs.python.org/3/library/datetime.html
# stackoverflow.com/questions/22715086/scheduling-python-script-to-run-every-hour-accurately

while 1:
	dt = datetime.now()
	dt = dt.replace(hour=0,minute=59)

	# if it's sometime after midnight...but before 1 AM
	if datetime.now() < dt:

		inFile = pd.read_csv('stock_performance.csv')

		for x in range(0, len(inFile['stock_ticker'])):


			def plot_results_triple(stkFile, npFinal, file_to_save, picName, predicted_data, true_data, prediction_len):

				df2 = pd.read_csv(file_to_save)
				for i in npFinal:
					df2 = df2.append({'Close':i}, ignore_index=True)

				b1 = len(df2)-150
				b2 = len(df2)-50
				g1 = len(df2)-51
				g2 = len(df2)
				blue = df2[(df2.index >= b1) & (df2.index < b2)]
				gold = df2[(df2.index >= g1) & (df2.index < g2)]

				fig = pyplot.figure(facecolor='white')
				ax = fig.add_subplot(111)
				
				pyplot.plot(gold.index, gold.Close, color='gold')
				ax.plot(blue.index, blue.Close, color='navy')

				dT1 = date.today() - timedelta(days = 150)
				dT2 = date.today()
				dT3 = date.today() + timedelta(days = 50)

				dTime1 = dt.datetime.strftime(dT1, '%Y-%m-%d')
				dTime2 = dt.datetime.strftime(dT2, '%Y-%m-%d')
				dTime3 = dt.datetime.strftime(dT3, '%Y-%m-%d')

				pyplot.text(b1, blue['Close'][b1], dTime1, weight='bold', va='bottom', fontsize=8, rotation=85)
				pyplot.text(b2, gold['Close'][g1], dTime2, weight='bold', va='bottom', fontsize=8, rotation=85)
				pyplot.text((g2-1), gold['Close'][g2-1], dTime3, weight='bold', va='bottom', fontsize=8, rotation=85)

				pyplot.xlabel('Trading Days')
				pyplot.ylabel('Price')

				ax1 = pyplot.axes()
				x_axis = ax1.axes.get_xaxis()
				x_axis.set_label_text('foo')
				x_axis.label.set_visible(False)
				x_axis.set_major_locator(ticker.NullLocator())
				x_axis.set_minor_locator(ticker.NullLocator())
				x_label = x_axis.get_label()
				x_label.set_visible(False)

				picPath = os.getcwd() + "/saved_pngs/"
				picName = picName + "2.png"
				os.chdir(picPath)
				pyplot.savefig(picName, dpi=400, bbox_inches='tight')
				os.chdir("../")



			def plot_results(picName, predicted_data, true_data):
				fig = pyplot.figure(facecolor='white')
				ax = fig.add_subplot(111)
				ax.plot(true_data, label='True Data')
				pyplot.plot(predicted_data, label='Prediction')
				# pyplot.legend()
				fig.autofmt_xdate()
				# pyplot.show()
				picPath = os.getcwd() + "/saved_pngs/"
				picName = picName + ".png"
				os.chdir(picPath)
				# pyplot.savefig(picName)
				pyplot.savefig(picName, dpi=400, bbox_inches='tight')
				os.chdir("../")


			def plot_results_multiple(picName, predicted_data, true_data, prediction_len):
				fig = pyplot.figure(facecolor='white')
				ax = fig.add_subplot(111)
				ax.plot(true_data, label='True Data')
				# Pad the list of predictions to shift it in the graph to it's correct start
				for i, data in enumerate(predicted_data):
					padding = [None for p in range((i * prediction_len))]
					pyplot.plot(padding + data, label='Prediction')
					# pyplot.legend()
				#pyplot.tight_layout()
				fig.autofmt_xdate()
				# pyplot.show()
				picPath = os.getcwd() + "/saved_pngs/"
				picName = picName + ".png"
				os.chdir(picPath)
				# pyplot.savefig(picName)
				# pyplot.savefig(picName, bbox_inches='tight')
				pyplot.savefig(picName, dpi=400, bbox_inches='tight')
				os.chdir("../")

			def save_future(ticker, finaloutput):			
				future = os.getcwd() + "/saved_results/"
				file = ticker + ".csv"
				df = pd.DataFrame(finaloutput)
				os.chdir(future)
				df.to_csv(file)
				os.chdir("../")

			def lstm(sTicker, postn, sFile, file_to_save):

				configs = json.load(open('config.json', 'r'))
				# print("hello")
				# "How can I safely create a nested directory in Python?"
				# stackoverflow.com/questions/273192/how-can-i-safely-create-a-nested-directory-in-python
				if not os.path.exists(configs['model']['save_dir']): os.makedirs(configs['model']['save_dir'])

				data = DataLoader(
					os.path.join(os.getcwd(), file_to_save),
					configs['data']['train_test_split'],
					configs['data']['columns']
				)

				try:
					model = Model()
					model.build_model(configs)
					x, y = data.get_train_data(
						seq_len=configs['data']['sequence_length'],
						normalise=configs['data']['normalise']
					)


					# out-of memory generative training
					steps_per_epoch = math.ceil((data.len_train - configs['data']['sequence_length']) / configs['training']['batch_size'])
					model.train_generator(
						data_gen=data.generate_train_batch(
							seq_len=configs['data']['sequence_length'],
							batch_size=configs['training']['batch_size'],
							normalise=configs['data']['normalise']
						),
						epochs=configs['training']['epochs'],
						batch_size=configs['training']['batch_size'],
						steps_per_epoch=steps_per_epoch,
						save_dir=configs['model']['save_dir'],
						file_name=sTicker
					)

					x_test, y_test = data.get_test_data(
						seq_len=configs['data']['sequence_length'],
						normalise=configs['data']['normalise']
					)

					predictions = model.predict_sequences_multiple(x_test, configs['data']['sequence_length'], configs['data']['sequence_length'])
					# predictions = model.predict_sequence_full(x_test, configs['data']['sequence_length'])
					# predictions = model.predict_point_by_point(x_test)
					prediction_data = predictions[-1:]
					prediction_data = prediction_data[0]
					normalizer = data.data_test[-50, [0]]
					# print(normalizer)
					finaloutput = []
					# print(prediction_data)
					for i in range(len(prediction_data)):
						finaloutput.append((prediction_data[i]+ 1)*normalizer[0])
					
					
					# print(finaloutput)
					numpyFinal = np.array(finaloutput)
					save_future(sTicker, numpyFinal)


					plot_results_triple(sFile, finaloutput, file_to_save, sTicker, predictions, y_test, configs['data']['sequence_length'])
					# plot_results_multiple(sTicker, predictions, y_test, configs['data']['sequence_length'])
					# plot_results(sTicker, predictions, y_test)


					totalDiff = [0] * len(predictions[0])
					for x in range(0,49):
						y = y_test[(len(y_test)-49)+x] - predictions[len(predictions) - 1][x]
						if y < 0.0:
							y *= -1
						totalDiff[x] = y
					stdDev = float(np.std(totalDiff, 0))

					nPred = predictions[len(predictions)-1]
					closeVal1 = pd.read_csv('stock_market_data.csv')
					closeVal2 = closeVal1['Close']
					closeVal3 = closeVal2[len(closeVal2)-1]

					avgPerc = 0
					for deviate in nPred:
						avgPerc += float(deviate)
					divisor = len(nPred)
					nMod1 = avgPerc / divisor

					latPerc = nPred[len(nPred)-1] - nPred[len(nPred)-2]
					nMod2 = nMod1 + 1
					nMod3 = latPerc + 1
					dPred1 = closeVal3 * nMod2
					dPred2 = closeVal3 * nMod3

					sFile['price_latest'][postn] = "{0:.2f}".format(closeVal3)
					sFile['percent_change_avg'][postn] = "{0:.6f}".format(nMod1)
					sFile['percent_change_latest'][postn] = "{0:.6f}".format(latPerc)
					sFile['prediction_by_avg'][postn] = "{0:.2f}".format(dPred1)
					sFile['prediction_by_latest'][postn] = "{0:.2f}".format(dPred2)
					sFile['standard_deviation'][postn] = "{0:.6f}".format(stdDev)

				except:
					sFile['price_latest'][postn] = "n/a"
					sFile['percent_change_avg'][postn] = "n/a"
					sFile['percent_change_latest'][postn] = "n/a"
					sFile['prediction_by_avg'][postn] = "n/a"
					sFile['prediction_by_latest'][postn] = "n/a"
					sFile['standard_deviation'][postn] = "n/a"

				updatedFile = pd.DataFrame({'stock_ticker':sFile['stock_ticker'],'percent_change_avg':sFile['percent_change_avg'],'percent_change_latest':sFile['percent_change_latest'],'prediction_by_avg':sFile['prediction_by_avg'],'prediction_by_latest':sFile['prediction_by_latest'],'price_latest':sFile['price_latest'],'standard_deviation':sFile['standard_deviation']})
				updatedFile.to_csv("stock_performance.csv",index=False,header=True)
			##
			##----------------------

			stockTicker = str(inFile['stock_ticker'][x])

			if stockTicker == "stock_ticker":
				#print("null")
				continue
			elif stockTicker ==  "'stock_ticker'":
				#print("null")
				continue
			elif stockTicker ==  "":
				#print("null")
				continue
			elif stockTicker is None:
				#print("null")
				continue
			else:

				data_source = 'alphavantage'
				api_key = 'MUI2CNDDJ6YV4WM7'
				url_string = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=%s&outputsize=full&apikey=%s"%(stockTicker,api_key)

				try:
					file_to_save = 'stock_market_data.csv'

					with urllib.request.urlopen(url_string) as url:
						data = json.loads(url.read().decode())
						# extract stock market data
						data = data['Time Series (Daily)']
						df = pd.DataFrame(columns=['Date','Low','High','Close','Open','Volume'])
						for k,v in data.items():
							date = dt.datetime.strptime(k, '%Y-%m-%d')
							data_row = [date.date(),float(v['3. low']),float(v['2. high']),
								float(v['4. close']),float(v['1. open']),float(v['5. volume'])]
							df.loc[-1,:] = data_row
							df.index = df.index + 1

					df.sort_index(ascending=True, inplace=True)
					df.to_csv(file_to_save)

					lstm(stockTicker, x, inFile, file_to_save)

					os.remove(file_to_save)

				except:
					inFile['price_latest'][x] = "n/a"
					inFile['percent_change_avg'][x] = "n/a"
					inFile['percent_change_latest'][x] = "n/a"
					inFile['prediction_by_avg'][x] = "n/a"
					inFile['prediction_by_latest'][x] = "n/a"
					inFile['standard_deviation'][x] = "n/a"

					updateFile = pd.DataFrame({'stock_ticker':inFile['stock_ticker'],'percent_change_avg':inFile['percent_change_avg'],'percent_change_latest':inFile['percent_change_latest'],'prediction_by_avg':inFile['prediction_by_avg'],'prediction_by_latest':inFile['prediction_by_latest'],'price_latest':inFile['price_latest'],'standard_deviation':inFile['standard_deviation']})
					updateFile.to_csv("stock_performance.csv",index=False,header=True)

		
	# sleep for one hour
	time.sleep(360)