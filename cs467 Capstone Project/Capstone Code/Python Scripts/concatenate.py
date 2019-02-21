from glob import glob
import glob

my_files = glob.glob("*.csv") 

header_saved = False
with open('StockData.csv','w') as fout:
    for filename in my_files:
        with open(filename) as fin:
            header = next(fin)
            if not header_saved:
                fout.write(header)
                header_saved = True
            for line in fin:
                fout.write(line)
