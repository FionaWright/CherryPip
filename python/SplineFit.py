import numpy as np
import pandas as pd
import sys
from scipy.interpolate import UnivariateSpline
from bokeh.plotting import figure, show, output_file
from bokeh.models import Legend
from scipy.interpolate import PPoly
from scipy.interpolate import LSQUnivariateSpline

# -------- CONFIG --------
LAMBDA_MIN = 390
LAMBDA_MAX = 780
LAMBDA_COUNT = 391
LAMBDA_DELTA = 1

# Spline smoothness (tune this!)
SPLINE_SMOOTHNESS = 1e-6

# -------- LOAD CSV --------
data = pd.read_csv(sys.argv[1])

X_data = data["X"].values
Y_data = data["Y"].values
Z_data = data["Z"].values

# Wavelengths
wavelengths = np.arange(LAMBDA_MIN, LAMBDA_MIN + LAMBDA_COUNT, LAMBDA_DELTA)

# Normalize wavelength (optional but fine)
wl_norm = (wavelengths - LAMBDA_MIN) / (LAMBDA_MAX - LAMBDA_MIN)

# -------- SPLINE FIT FUNCTION --------
def fit_channel(y_data, name):
    NUM_KNOTS = 10
    knots = np.linspace(0.05, 0.95, NUM_KNOTS)
    spline = LSQUnivariateSpline(wl_norm, y_data, knots, k=3)

    fitted = spline(wl_norm)
    rmse = np.sqrt(np.mean((fitted - y_data) ** 2))

    print(f"\n{name} channel:")
    print(f"RMSE: {rmse:.6f}")

    # ---- EXTRACT SPLINE DATA ----
    knots = spline.get_knots()
    coeffs = spline.get_coeffs()
    degree = 3

    print(f"\n{name} spline parameters:")
    print(f"Degree: {degree}")
    print(f"Knots ({len(knots)}):")
    print(", ".join(f"{k:.6f}" for k in knots))

    print(f"\nCoefficients ({len(coeffs)}):")
    print(", ".join(f"{c:.6f}" for c in coeffs))

    return spline, fitted, knots, coeffs, degree

# -------- RUN FITS --------
spline_X, fit_X, knots_X, coeffs_X, deg_X = fit_channel(X_data, "X")
spline_Y, fit_Y, knots_Y, coeffs_Y, deg_Y = fit_channel(Y_data, "Y")
spline_Z, fit_Z, knots_Z, coeffs_Z, deg_Z = fit_channel(Z_data, "Z")

def spline_to_ppoly(spline):
    ppoly = PPoly.from_spline(spline._eval_args)
    return ppoly

pp_X = spline_to_ppoly(spline_X)
pp_Y = spline_to_ppoly(spline_Y)
pp_Z = spline_to_ppoly(spline_Z)

def print_ppoly(pp, name):
    print(f"\n{name} piecewise polynomial:")

    breaks = pp.x  # interval boundaries
    coeffs = pp.c  # shape: (degree+1, intervals)

    degree = coeffs.shape[0] - 1
    intervals = coeffs.shape[1]

    print(f"Intervals: {intervals}")
    print(f"Degree: {degree}")

    for i in range(intervals):
        print(f"\n// Interval {i}: [{breaks[i]:.6f}, {breaks[i+1]:.6f}]")
        print("{")
        for d in range(degree + 1):
            print(f"  {coeffs[d, i]:.8e},")
        print("}")

print_ppoly(pp_X, "X")
print_ppoly(pp_Y, "Y")
print_ppoly(pp_Z, "Z")

# -------- VALIDATION --------
print("\n--- Validation ---")
def validate_channel(y_data, fitted, name):
    max_error = np.max(np.abs(fitted - y_data))
    mean_error = np.mean(np.abs(fitted - y_data))
    print(f"{name}: Max error = {max_error:.6f}, Mean error = {mean_error:.6f}")

validate_channel(X_data, fit_X, "X")
validate_channel(Y_data, fit_Y, "Y")
validate_channel(Z_data, fit_Z, "Z")

# -------- BOKEH PLOT --------
output_file("cie_fit.html", title="CIE XYZ Spline Fit")

p = figure(title="CIE XYZ Curve Fitting (Spline)",
           x_axis_label='Wavelength (nm)',
           y_axis_label='Value',
           width=900, height=500)

# Original
orig_lines = [
    p.line(wavelengths, X_data, color="red", line_width=2),
    p.line(wavelengths, Y_data, color="green", line_width=2),
    p.line(wavelengths, Z_data, color="blue", line_width=2)
]

# Fitted
fit_lines = [
    p.line(wavelengths, fit_X, color="red", line_width=2, line_dash="dashed"),
    p.line(wavelengths, fit_Y, color="green", line_width=2, line_dash="dashed"),
    p.line(wavelengths, fit_Z, color="blue", line_width=2, line_dash="dashed")
]

legend_items = [
    ("X original", [orig_lines[0]]),
    ("Y original", [orig_lines[1]]),
    ("Z original", [orig_lines[2]]),
    ("X fitted", [fit_lines[0]]),
    ("Y fitted", [fit_lines[1]]),
    ("Z fitted", [fit_lines[2]]),
]

legend = Legend(items=legend_items, location="top_right")
p.add_layout(legend)

show(p)