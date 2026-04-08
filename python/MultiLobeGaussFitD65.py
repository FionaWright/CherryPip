import numpy as np
from scipy.optimize import curve_fit
import sys
from bokeh.plotting import figure, show, output_file

# -------- CONFIG --------
NUM_GAUSSIANS = 7

LAMBDA_MIN = 300.0
LAMBDA_MAX = 830.0
LAMBDA_COUNT = 531

# -------- LOAD DATA --------
# Expect raw float array (one value per line OR comma-separated)
data = np.loadtxt(sys.argv[1], delimiter=',')

assert len(data) == LAMBDA_COUNT, "Data size mismatch"

# Reconstruct wavelengths
wavelengths = np.linspace(LAMBDA_MIN, LAMBDA_MAX, LAMBDA_COUNT)

# Normalize wavelength
wl_norm = (wavelengths - LAMBDA_MIN) / (LAMBDA_MAX - LAMBDA_MIN)

# -------- GAUSSIAN MODEL --------
def gaussian(x, mu, sigma):
    return np.exp(-0.5 * ((x - mu) / sigma) ** 2)

def multi_gaussian(x, *params):
    result = np.zeros_like(x)
    for i in range(0, len(params), 3):
        a = params[i]
        mu = params[i + 1]
        sigma = params[i + 2]
        result += a * gaussian(x, mu, sigma)
    return result

# -------- FIT --------
def fit_d65(y_data):
    params_init = []

    for i in range(NUM_GAUSSIANS):
        mu_guess = (i + 0.5) / NUM_GAUSSIANS
        sigma_guess = 0.15
        a_guess = np.max(y_data) / NUM_GAUSSIANS
        params_init += [a_guess, mu_guess, sigma_guess]

    lower = [0.0, 0.0, 0.01] * NUM_GAUSSIANS
    upper = [np.inf, 1.0, 0.5] * NUM_GAUSSIANS

    params, _ = curve_fit(
        multi_gaussian,
        wl_norm,
        y_data,
        p0=params_init,
        bounds=(lower, upper),
        maxfev=50000
    )

    fitted = multi_gaussian(wl_norm, *params)

    max_error = np.max(np.abs(fitted - y_data))
    mean_error = np.mean(np.abs(fitted - y_data))

    print("\n--- D65 Fit ---")
    print(f"Max error: {max_error:.6f}")
    print(f"Mean error: {mean_error:.6f}")

    for i in range(0, len(params), 3):
        print(f"  {params[i]:.6f}, {params[i+1]:.6f}, {params[i+2]:.6f}")

    return params, fitted

# -------- RUN --------
params, fitted = fit_d65(data)

# -------- PLOT --------
output_file("d65_fit.html")

p = figure(title="D65 Gaussian Fit",
           x_axis_label='Wavelength (nm)',
           y_axis_label='Intensity',
           width=900, height=500)

p.line(wavelengths, data, line_width=2, legend_label="Original")
p.line(wavelengths, fitted, line_width=2, line_dash="dashed", legend_label="Fitted")

p.legend.location = "top_right"

show(p)

# -------- HLSL EXPORT --------
print("\n// HLSL Multi-Lobe Gaussian (D65)")
print("static const float3 D65_PARAMS[] = {")

for i in range(0, len(params), 3):
    a, mu, sigma = params[i], params[i+1], params[i+2]
    print(f"    float3({a:.6f}, {mu:.6f}, {sigma:.6f}),")

print("};")