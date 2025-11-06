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

plot_histogram(data, num_bins=255, name="histogram_test", show_plot=show_plot, save_plot=save_plot)