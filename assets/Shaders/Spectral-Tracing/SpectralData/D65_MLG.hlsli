#ifndef H_D65_MLG_H
#define H_D65_MLG_H

#include "Spectral-Tracing/SpectralData/MultiLobeGauss.hlsli"

// D65 Standard Illuminant Multi-Lobe Gaussian Curve Fit

static const MultiLobeGauss cWhiteD65_MLG = {
    { 60.978208, 0.585681, 0.160422 },
    { 23.980888, 0.059189, 0.021373 },
    { 45.776326, 0.201340, 0.022717 },
    { 57.635830, 0.277625, 0.050593 },
    { 45.151910, 0.123659, 0.039862 },
    { 69.992053, 0.380150, 0.103458 },
    { 54.459604, 0.978976, 0.258670 },
};

#endif