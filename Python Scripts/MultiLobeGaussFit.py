import numpy as np
import pandas as pd
from scipy.optimize import curve_fit
import sys
from bokeh.plotting import figure, show, output_file
from bokeh.models import Legend

# -------- CONFIG --------
NUM_GAUSSIANS = 2  # per channel
NUM_SIGMOIDS = 2
LAMBDA_MIN = 390
LAMBDA_MAX = 780
LAMBDA_COUNT = 391
LAMBDA_DELTA = 1

# -------- LOAD CSV --------
# CSV must have columns: X,Y,Z (no wavelength)
data = pd.read_csv(sys.argv[1])

X_data = data["X"].values
Y_data = data["Y"].values
Z_data = data["Z"].values

# Reconstruct wavelength array
wavelengths = np.arange(LAMBDA_MIN, LAMBDA_MIN + LAMBDA_COUNT, LAMBDA_DELTA)

# Normalize wavelength to [0,1] for stable fitting
wl_norm = (wavelengths - LAMBDA_MIN) / (LAMBDA_MAX - LAMBDA_MIN)

# -------- GAUSSIAN MODEL --------
def gaussian(x, mu, sigma):
    return np.exp(-0.5 * ((x - mu) / sigma) ** 2)

def sigmoid(x, k, x0):
    return 1.0 / (1.0 + np.exp(-k * (x - x0)))

def multi_gaussian(x, *params):
    c = params[0]  # baseline
    result = np.full_like(x, c)

    for i in range(1, len(params), 3):
        a = params[i]
        mu = params[i + 1]
        sigma = params[i + 2]
        result += a * gaussian(x, mu, sigma)

    return result

def model(x, *params):
    idx = 0

    # baseline
    c = params[idx]
    idx += 1
    result = np.full_like(x, c)

    # gaussians
    for _ in range(NUM_GAUSSIANS):
        a = params[idx]
        mu = params[idx + 1]
        sigma = params[idx + 2]
        result += a * gaussian(x, mu, sigma)
        idx += 3

    # sigmoids
    for _ in range(NUM_SIGMOIDS):
        b = params[idx]
        k = params[idx + 1]
        x0 = params[idx + 2]
        result += b * sigmoid(x, k, x0)
        idx += 3

    return result

# -------- FIT FUNCTION --------
def fit_channel(y_data, name):
    params_init = [np.min(y_data)]

    # gaussians
    for i in range(NUM_GAUSSIANS):
        params_init += [
            (np.max(y_data) - np.min(y_data)) / NUM_GAUSSIANS,
            (i + 0.5) / NUM_GAUSSIANS,
            0.2
        ]

    # sigmoids
    for i in range(NUM_SIGMOIDS):
        params_init += [
            0.1,   # amplitude
            10.0,  # steepness
            0.5    # center
        ]

    lower = [0.0]
    upper = [1.0]

    # gaussians
    for _ in range(NUM_GAUSSIANS):
        lower += [0.0, 0.0, 0.01]
        upper += [np.inf, 1.0, 1.0]

    # sigmoids
    for _ in range(NUM_SIGMOIDS):
        lower += [-1.0, 0.1, 0.0]   # b, k, x0
        upper += [1.0, 50.0, 1.0]

    params, _ = curve_fit(
        model,
        wl_norm,
        y_data,
        p0=params_init,
        bounds=(lower, upper),
        maxfev=50000
    )

    fitted = model(wl_norm, *params)
    rmse = np.sqrt(np.mean((fitted - y_data) ** 2))

    print(f"\n{name} channel:")
    print(f"RMSE: {rmse:.6f}")
    idx = 0
    print(f"  baseline: {params[idx]:.6f}")
    idx += 1

    print("  Gaussians:")
    for _ in range(NUM_GAUSSIANS):
        print(f"    a={params[idx]:.6f}, mu={params[idx+1]:.6f}, sigma={params[idx+2]:.6f}")
        idx += 3

    print("  Sigmoids:")
    for _ in range(NUM_SIGMOIDS):
        print(f"    b={params[idx]:.6f}, k={params[idx+1]:.6f}, x0={params[idx+2]:.6f}")
        idx += 3

    return params, fitted

# -------- RUN FITS --------
params_X, rmse_X = fit_channel(X_data, "X")
params_Y, rmse_Y = fit_channel(Y_data, "Y")
params_Z, rmse_Z = fit_channel(Z_data, "Z")

# -------- SIMPLE VALIDATION --------
print("\n--- Validation ---")
def validate_channel(y_data, params, name):
    fitted = model(wl_norm, *params)
    max_error = np.max(np.abs(fitted - y_data))
    mean_error = np.mean(np.abs(fitted - y_data))
    print(f"{name}: Max error = {max_error:.6f}, Mean error = {mean_error:.6f}")

validate_channel(X_data, params_X, "X")
validate_channel(Y_data, params_Y, "Y")
validate_channel(Z_data, params_Z, "Z")

# -------- BOKEH PLOT --------
output_file("cie_fit.html", title="CIE XYZ Gaussian Fit")

p = figure(title="CIE XYZ Curve Fitting",
           x_axis_label='Wavelength (nm)',
           y_axis_label='Value',
           width=900, height=500)

# Original data
orig_lines = [
    p.line(wavelengths, X_data, color="red", line_width=2),
    p.line(wavelengths, Y_data, color="green", line_width=2),
    p.line(wavelengths, Z_data, color="blue", line_width=2)
]

# Fitted curves (dashed)
fit_lines = [
    p.line(wavelengths, rmse_X, color="red", line_width=2, line_dash="dashed"),
    p.line(wavelengths, rmse_Y, color="green", line_width=2, line_dash="dashed"),
    p.line(wavelengths, rmse_Z, color="blue", line_width=2, line_dash="dashed")
]

# Legend
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