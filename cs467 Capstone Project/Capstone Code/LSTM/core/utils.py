#Credit to "Jakob Aungiers" https://github.com/jaungiers/LSTM-Neural-Network-for-Time-Series-Prediction
import datetime as dt

class Timer():

	def __init__(self):
		self.start_dt = None

	def start(self):
		self.start_dt = dt.datetime.now()

	def stop(self):
		end_dt = dt.datetime.now()
		print('Time taken: %s' % (end_dt - self.start_dt))