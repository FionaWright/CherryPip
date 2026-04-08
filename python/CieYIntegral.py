import csv
import sys

delta_lambda = 1.0
Y = []

with open(sys.argv[1]) as f:
    reader = csv.reader(f)
    for row in reader:
        if len(row) < 3:
            continue
        try:
            y = float(row[1])   # second column = Ybar
            Y.append(y)
        except ValueError:
            continue

cie_y_integral = sum(Y) * delta_lambda

print("CIE Y integral =", cie_y_integral)
print("Normalization constant k =", 1.0 / cie_y_integral)