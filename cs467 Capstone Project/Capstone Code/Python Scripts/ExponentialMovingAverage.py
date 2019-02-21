from pandas_datareader import data
import matplotlib.pyplot as plt
import pandas as pd
import datetime as dt
import urllib.request, json
import os
import numpy as np
import tensorflow as tf # This code has been tested with TensorFlow 1.6
from sklearn.preprocessing import MinMaxScaler

data_source = 'alphavantage' # alphavantage or kaggle
# ====================== Loading Data from Alpha Vantage ==================================

api_key = '8UB1ISP9VM4F2G4J'

# American Airlines stock market prices
ticker = "IBM"

# JSON file with all the stock market data for AAL from the last 20 years
url_string = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=%s&outputsize=full&apikey=%s"%(ticker,api_key)

# Save data to this file
file_to_save = 'stock_market_data-%s.csv'%ticker

# If you haven't already saved data,
# Go ahead and grab the data from the url
# And store date, low, high, volume, close, open values to a Pandas DataFrame
if not os.path.exists(file_to_save):
    with urllib.request.urlopen(url_string) as url:
        data = json.loads(url.read().decode())
        # extract stock market data
        data = data['Time Series (Daily)']
        df = pd.DataFrame(columns=['Date','Low','High','Close','Open'])
        for k,v in data.items():
            date = dt.datetime.strptime(k, '%Y-%m-%d')
            data_row = [date.date(),float(v['3. low']),float(v['2. high']),
                float(v['4. close']),float(v['1. open'])]
            df.loc[-1,:] = data_row
            df.index = df.index + 1     
    df.to_csv(file_to_save)

# If the data is already there, just load it from the CSV
else:
    df = pd.read_csv(file_to_save)


# Sort DataFrame by date
df = df.sort_values('Date')

# Double check the result
df.head()

# First calculate the mid prices from the highest and lowest
high_prices = df.loc[:,'High'].as_matrix()
low_prices = df.loc[:,'Low'].as_matrix()
mid_prices = (high_prices+low_prices)/2.0
close_prices = df.loc[:,'Close'].as_matrix()
length = len(close_prices) - 1
trainNum = length * 0.75
testNum = length - trainNum

# Now perform exponential moving average smoothing
# So the data will have a smoother curve than the original ragged data
EMA = 0.0
gamma = 0.1
for ti in range(length):
  EMA = gamma*close_prices[ti] + (1-gamma)*EMA
  close_prices[ti] = EMA

window_size = 100
std_avg_predictions = []
std_avg_x = []
mse_errors = []

for pred_idx in range(window_size,length):

    date = df.loc[pred_idx,'Date']
    std_avg_predictions.append(np.mean(close_prices[pred_idx-window_size:pred_idx]))
    std_avg_x.append(date)

plt.figure(figsize = (18,9))
plt.plot(range(df.shape[0]),close_prices,color='b',label='True')
plt.plot(range(window_size,length),std_avg_predictions,color='orange',label='Prediction')
plt.xticks(range(0,df.shape[0],50),df['Date'].loc[::50],rotation=45)
plt.xlabel('Date')
plt.ylabel('Mid Price')
plt.title('Moving Average')
plt.legend(fontsize=18)
plt.show()

window_size = 100
run_avg_predictions = []
run_avg_x = []

mse_errors = []

running_mean = 0.0
run_avg_predictions.append(running_mean)

decay = 0.5

for pred_idx in range(1,length):
    running_mean = running_mean*decay + (1.0-decay)*close_prices[pred_idx-1]
    run_avg_predictions.append(running_mean)
    run_avg_x.append(date)


plt.figure(figsize = (18,9))
plt.plot(range(df.shape[0]),close_prices,color='b',label='True')
plt.plot(range(0,length),run_avg_predictions,color='orange', label='Prediction')
plt.xticks(range(0,df.shape[0],50),df['Date'].loc[::50],rotation=45)
plt.xlabel('Date')
plt.ylabel('Mid Price')
plt.title('Exponential Moving Average')
plt.legend(fontsize=18)
plt.show()
