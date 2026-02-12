from bokeh.plotting import figure, show, output_file, save
from bokeh.models import ColumnDataSource
import numpy as np
from collections import Counter
from bokeh.palettes import Category10

def plot_histogram(x_data: list[float], num_bins: int, name: str, show_plot: bool, save_plot: bool):
    """
    Plots a histogram using Bokeh.

    Parameters:
    xData: list of x-values
    yData: list of y-values
    numBins: number of bins
    name: file name for saving the plot (without extension)
    show_plot: whether to display the plot
    save_plot: whether to save the plot as an HTML file
    """
    hist, edges = np.histogram(x_data, bins=num_bins)

    p = figure(title="Histogram", x_axis_label='x', y_axis_label='y')
    p.quad(top=hist, bottom=0, left=edges[:-1], right=edges[1:], fill_color="navy", line_color="white", alpha=0.7)

    output_file(f"BokehData/{name}.html")
    if save_plot:
        save(p)

    if show_plot:
        show(p)

def plot_value_counts(x_data: list[float], name: str, show_plot: bool, save_plot: bool):
    counts = Counter(x_data)            # count occurrences of each unique value
    values = list(counts.keys())        # x positions
    freqs = list(counts.values())       # heights of bars

    p = figure(title=name, x_axis_label='x', y_axis_label='y')
    p.vbar(x=values, top=freqs, width=0.8, fill_color="navy", line_color="white", alpha=0.7)

    output_file(f"BokehData/{name}.html")
    if save_plot:
        save(p)
    if show_plot:
        show(p)

def plot_line(data: list[float], name: str, show_plot: bool, save_plot: bool):
    x = list(range(len(data)))  # index (e.g., iteration or epoch)

    p = figure(title=name, x_axis_label="Frame", y_axis_label="RMSE")
    p.line(x, data, line_width=2)
    p.scatter(x, data, size=6)

    output_file(f"BokehData/{name}.html")
    if save_plot:
        save(p)
    if show_plot:
        show(p)

def plot_multiple_lines(data_lists: list[list[float]], names: list[str], show_plot: bool, save_plot: bool):
    p = figure(title="Convergence Graph against Golden Image", x_axis_label="Frame", y_axis_label="RMSE")

    palette = Category10[10]  # Up to 10 distinct colors
    for i, (data, name) in enumerate(zip(data_lists, names)):
        x = list(range(len(data)))
        color = palette[i % len(palette)]
        p.line(x, data, line_width=2, color=color, legend_label=name)
        p.scatter(x, data, size=6, color=color)

    p.legend.location = "top_left"
    output_file("BokehData/multiple_lines_plot.html")

    if save_plot:
        save(p)
    if show_plot:
        show(p)