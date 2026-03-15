#ifndef H_FRESNEL_SPECTRAL_H
#define H_FRESNEL_SPECTRAL_H

// Nabla Implementation: (Dielectric only, No Conductor)
// https://github.com/Devsh-Graphics-Programming/Nabla/blob/82726cbac7e18f702dfebdacd862697216e9b0fd/include/nbl/builtin/hlsl/bxdf/fresnel.hlsl

// Theory:
// https://en.wikipedia.org/wiki/Maxwell's_equations
// https://em.geosci.xyz/content/maxwell1_fundamentals/reflection_and_refraction/Fresnel_equations.html
// https://pbr-book.org/4ed/Reflection_Models/Specular_Reflection_and_Transmission
// https://scispace.com/pdf/handbook-of-optical-materials-3e2ukwmx2l.pdf

// Eta Tables:
// https://github.com/polyanskiy/refractiveindex.info-database/tree/main/database
//     Data Columns: λ, n(λ), k(λ)
//     Wavelength is in micrometers, multiply by 1000 to get nm
//     Not all materials contain data in the visible light spectrum
// https://refractiveindex.info/

/*

E = Electric Field
H = Magnetic Field Intensity
Together they form an electromagnetic wave (light)
For plane waves E, H, and rayDir are all perpendicular
There's also B = Magnetic Flux Density which is proportional to H

Polarization describes the orientation of E as the light propogates
Linear Polarization: E oscillates along one fixed direction
Circle Polarization: E moves in a circle
Elliptical Polarization: General case, E traces an ellipse
Unpolarized: A mixture of many polarization states
Assume unpolarized for all rendering unless you are specifically implementing polarized rendering

S-Polarization (Rs) is when E is oriented perpendicular to the plane of incidence
P-Polarization (Rp) is when E is parallel
Fresnel Unpolarized Reflectance is (Rs + Rp) / 2

The Maxwell Equations define certain conditions of the E and H fields to be continuous across the interface between two media

The Fresnel Equations solve the Maxwell Equations for a plane wave hitting a flat interface
The reflected & transmitted field amplitudes are given as functions of:
- Incident Angle (cosTheta or NdH)
- Polarization
- The IORs
- Wavelength

Fresnel is an approximation of Maxwell
Schlick is an approximation of Fresnel

Snell's Law: η_1 sin(θ_1) = η_2 sin(θ_2)
Used to compute the refraction direction from IOR

Try to avoid mixing up absolute vs relative IOR
n is the abolute IOR against a vacuum
eta is (often) the relative IOR (n_2/n_1)

Dielectrics have a simple IOR spectra. Conductors instead need complex IOR spectra
Conductor IOR: n(λ) + ik(λ)
n is the phase velocity, k is the extinction coefficient describing absorption

*/

// TODO: Move data to CPU / Structured Buffer / Material Index
#include "IOR Tables/TiO2.h"
float DebugSampleIorN(float lambda)
{
    float fIdx = (lambda - IOR_TABLE_TIO2_LAMBDA_MIN) / IOR_TABLE_TIO2_LAMBDA_DELTA;
    fIdx = clamp(fIdx, 0.0f, IOR_TABLE_TIO2_COUNT-1);
    int i0 = (int)floor(fIdx);
    int i1 = min(i0+1, IOR_TABLE_TIO2_COUNT-1);
    float t = fIdx - i0;

    float sampleN0 = cIorTable_TiO2[i0].N;
    float sampleN1 = cIorTable_TiO2[i1].N;
    return lerp(sampleN0, sampleN1, t);
}

float SampleEta(float lambda, bool entering)
{
    float nCurrent = entering ? 1.5f : DebugSampleIorN(lambda);
    float nNext = entering ? DebugSampleIorN(lambda) : 1.5f;
    return nCurrent / nNext;
}

void Dielectric_Polarized(float eta2, float cosTheta, out float rp2, out float rs2)
{
    float sinTheta2 = 1.0f - cosTheta * cosTheta;

    // the max() clamping can handle TIR when eta2<1.0
    float t0 = sqrt(max(0.0f, eta2 - sinTheta2));
    float t2 = eta2 * cosTheta;

    float rp = (t0 - t2) / max(1e-6f, t0 + t2);
    rp2 = rp * rp;

    float rs = (cosTheta - t0) / max(1e-6f, cosTheta + t0);
    rs2 = rs * rs;
}

// Assumes eta is already flipped if entering
float Dielectric_Unpolarized(float eta2, float cosTheta)
{
    cosTheta = saturate(cosTheta);

    float rp2, rs2;
    Dielectric_Polarized(eta2, cosTheta, rp2, rs2);

    float reflectanceProb = (rp2 + rs2) * 0.5f;
    return reflectanceProb;
}

#endif