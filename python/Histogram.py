import csv
import os
import sys

from Utils.BokehPlot import *

os.chdir(os.path.dirname(os.path.abspath(__file__))) # Change CWD to "Python Scripts"

data = []

with open(sys.argv[1], newline='') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        data.append(float(row['data']))

show_plot = sys.argv.__contains__('--show')
save_plot = sys.argv.__contains__('--save')

import numpy as np

std = np.std(data, ddof=0)   # population standard deviation
mean = np.mean(data)
print(std, mean)

plot_value_counts(data, name=f"histogram_test (std={std}, mean={mean})", show_plot=show_plot, save_plot=save_plot)