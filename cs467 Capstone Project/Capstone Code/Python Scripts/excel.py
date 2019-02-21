import pandas as pd
stcktk = 'DJI'
name = 'Dow Jone Industrial Average'

file = pd.read_csv("%s.csv" %stcktk)
file['Name'] = name
file['StockTick'] = stcktk
keep_columns = ['Name', 'StockTick','Date','Open','High', 'Low', 'Close','Volume']
new_file = file[keep_columns]
new_file = new_file.round({'Open': 2, 'High': 2, 'Low': 2, 'Close': 2})
new_file.to_csv(("%s.csv" %stcktk), index = False)

