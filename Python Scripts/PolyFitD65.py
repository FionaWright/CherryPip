import numpy as np
from numpy.polynomial.chebyshev import Chebyshev
from bokeh.plotting import figure, show, output_file
import sys

# -------- CONFIG --------
DEGREE = 15
LAMBDA_MIN = 300.0
LAMBDA_MAX = 830.0
LAMBDA_COUNT = 531

# -------- LOAD DATA --------
# CSV or TXT: one row of floats, 531 entries
data = np.loadtxt(sys.argv[1], delimiter=',')
assert len(data) == LAMBDA_COUNT, "Data size mismatch"

# Wavelength array
wavelengths = np.linspace(LAMBDA_MIN, LAMBDA_MAX, LAMBDA_COUNT)

# Normalize to [-1,1] for Chebyshev fitting
t = 2*(wavelengths - LAMBDA_MIN)/(LAMBDA_MAX - LAMBDA_MIN) - 1

# -------- FIT CHEBYSHEV --------
cheb_fit = Chebyshev.fit(t, data, DEGREE)
fitted = cheb_fit(t)

# -------- ERROR --------
max_error = np.max(np.abs(fitted - data))
mean_error = np.mean(np.abs(fitted - data))
print(f"\n--- D65 Chebyshev Fit ---")
print(f"Max error: {max_error:.6f}")
print(f"Mean error: {mean_error:.6f}")

# -------- PLOT --------
output_file("d65_chebyshev_fit.html")

p = figure(title="D65 Chebyshev Polynomial Fit",
           x_axis_label='Wavelength (nm)',
           y_axis_label='Intensity',
           width=900, height=500)

p.line(wavelengths, data, line_width=2, color="blue", legend_label="Original")
p.line(wavelengths, fitted, line_width=2, line_dash="dashed", color="red", legend_label=f"Degree {DEGREE} Chebyshev Fit")

p.legend.location = "top_right"
show(p)

# -------- HLSL EXPORT --------
# Chebyshev coefficients: c0 + c1*T1(t) + ... + cN*TN(t)
print("\n// HLSL Chebyshev Coefficients (D65)")
coeffs = cheb_fit.convert().coef  # convert to standard polynomial basis
for i, c in enumerate(coeffs):
    print(f"static const float D65_C{i} = {c:.8f}f;")

print("\n// HLSL eval function:")
print("""
float EvalD65(float lambda)
{
    // Normalize lambda to [-1,1]
    float t = 2.0*(lambda - {lmin})/({lmax}-{lmin}) - 1.0;
    float y = 0.0;
""".format(lmin=LAMBDA_MIN, lmax=LAMBDA_MAX))

for i, c in enumerate(coeffs):
    print(f"    y += D65_C{i} * pow(t, {i});")

print("""
    return y;
}
""")