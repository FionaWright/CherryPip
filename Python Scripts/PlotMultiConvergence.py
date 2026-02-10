import csv
import os
import sys
import numpy as np

from Utils.BokehPlot import *

os.chdir(os.path.dirname(os.path.abspath(__file__))) # Change CWD to "Python Scripts"

rmseDataLists = []
testNames = []

for testName in sys.argv[1:]:
    rmseData = []
    testNames.append(os.path.splitext(os.path.basename(testName))[0])
    with open(testName, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            rmseData.append(float(row['RMSE']))
    rmseDataLists.append(rmseData)

show_plot = True
save_plot = True

plot_multiple_lines(rmseDataLists, testNames, show_plot=show_plot, save_plot=save_plot)