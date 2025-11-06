import csv
import sys

from Utils.BokehPlot import *

x_data = []
y_data = []

with open(sys.argv[1], newline='') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        x_data.append(float(row['x']))
        y_data.append(float(row['y']))

plot_histogram(x_data, y_data, num_bins=10, name="histogram_test", show_plot=True, save_plot=False)