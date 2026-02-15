import csv
import os
import sys
import numpy as np

from Utils.BokehPlot import *

os.chdir(os.path.dirname(os.path.abspath(__file__))) # Change CWD to "Python Scripts"

data = []

with open(sys.argv[1], newline='') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        data.append((float(row['X']), float(row['Y'])))

show_plot = sys.argv.__contains__('--show')
save_plot = sys.argv.__contains__('--save')

plot_points_2d(data, name=f"2D Points", show_plot=show_plot, save_plot=save_plot)