import numpy as np
from numpy.polynomial.chebyshev import Chebyshev
from bokeh.plotting import figure, show, output_file
import sys

# -------- CONFIG --------
DEGREE = 15  # degree per segment
LAMBDA_MIN = 390.0
LAMBDA_MAX = 780.0
LAMBDA_COUNT = 391

# -------- SPLIT CONFIG --------
# Indices where to split the data (0-based). Must be increasing and within [0, LAMBDA_COUNT-1].
# Example: split into 3 segments at indices 130, 260
SPLIT_INDICES = [560-390]

# -------- LOAD DATA --------
data = np.loadtxt(sys.argv[1], delimiter=',')[:, 0]  # select column
assert len(data) == LAMBDA_COUNT, "Data size mismatch"

wavelengths = np.linspace(LAMBDA_MIN, LAMBDA_MAX, LAMBDA_COUNT)

# -------- DETERMINE SEGMENTS --------
segments = []
start_idx = 0
for idx in SPLIT_INDICES + [LAMBDA_COUNT]:
    segments.append((start_idx, idx))
    start_idx = idx

# -------- FIT AND EXPORT --------
all_fits = []

print("\n--- Chebyshev Segment Fits ---")
for seg_id, (start, end) in enumerate(segments):
    seg_wl = wavelengths[start:end]
    seg_data = data[start:end]

    # normalize to [-1,1] over the segment
    t = 2*(seg_wl - seg_wl[0]) / (seg_wl[-1] - seg_wl[0]) - 1

    # fit Chebyshev
    cheb_fit = Chebyshev.fit(t, seg_data, DEGREE)
    fitted = cheb_fit(t)

    # store
    all_fits.append((seg_wl, fitted, cheb_fit.convert().coef, seg_wl[0], seg_wl[-1]))

    # print error
    max_error = np.max(np.abs(fitted - seg_data))
    mean_error = np.mean(np.abs(fitted - seg_data))
    print(f"Segment {seg_id}: {start}-{end-1}, Max error={max_error:.6f}, Mean error={mean_error:.6f}")

    # export HLSL coefficients
    coeffs = cheb_fit.convert().coef
    print(f"\n// HLSL Chebyshev Coefficients Segment {seg_id}")
    for i, c in enumerate(coeffs):
        print(f"static const float D65_C{seg_id}_{i} = {c:.8f}f;")

    # export HLSL eval function
    print(f"""
float EvalD65_Seg{seg_id}(float lambda)
{{
    float t = 2.0*(lambda - {seg_wl[0]})/({seg_wl[-1]}-{seg_wl[0]}) - 1.0;
    float y = 0.0;
""")
    for i, c in enumerate(coeffs):
        print(f"    y += D65_C{seg_id}_{i} * pow(t, {i});")
    print("    return y;\n}\n")

# -------- PLOT --------
output_file("d65_chebyshev_segments.html")
p = figure(title="D65 Chebyshev Segment Fit",
           x_axis_label='Wavelength (nm)',
           y_axis_label='Intensity',
           width=900, height=500)
p.line(wavelengths, data, line_width=2, color="blue", legend_label="Original")
colors = ["red", "green", "orange"]
for seg_id, (seg_wl, fitted, _, _, _) in enumerate(all_fits):
    p.line(seg_wl, fitted, line_width=2, line_dash="dashed", color=colors[seg_id%len(colors)], legend_label=f"Segment {seg_id} Fit")
p.legend.location = "top_right"
show(p)