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
// https://en.wikipedia.org/wiki/Sellmeier_equation

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

#ifndef IOR_AIR
#define IOR_AIR 1.0f
#endif

struct SellmeierEquation
{
    float Constant;
    float B[5];
    float C[5];
    bool MulLambda[5];

    float SampleN(float lambda_nano)
    {
        float lambda_micro = lambda_nano / 1000.0f;
        float lambda2 = lambda_micro * lambda_micro;

        float n2 = Constant;
        [unroll]
        for (int i = 0; i < 5; i++)
        {
            float term = B[i] / (lambda2 - C[i]);
            if (MulLambda[i])
                term *= lambda2;
            n2 += term;
        }
        return sqrt(n2);
    }
};

static const SellmeierEquation cSellmeier_TiO2 = {
    5.913f,
    { 0.2441f, 0, 0, 0, 0 },
    { 0.0803f, 0, 0, 0, 0 },
    { false, false, false, false, false }
};

static const SellmeierEquation cSellmeier_FusedSilica = {
    1.0f,
    { 0.6961663, 0.4079426, 0.8974794, 0, 0 },
    { 4.679148e-3, 0.01351206, 97.93400254, 0, 0 },
    { true, true, true, false, false }
};

static const SellmeierEquation cSellmeier_BGG = {
    1.0f,
    { 0.82725, 1.14635, 1.53923, 0, 0 },
    { 0.02978, 0.005117, 272.657, 0, 0 },
    { true, true, true, false, false }
};

float DebugSampleIorN(float lambda)
{
    return cSellmeier_BGG.SampleN(lambda);
}

// TODO: Temp
float DebugSampleIorN_Hero(SpectralContext ctx)
{
#if defined(SPECTRAL_SINGLE_WAVELENGTH_SAMPLING)
    return DebugSampleIorN(ctx.Lambda);
#elif defined(SPECTRAL_HERO_SAMPLING)
    return DebugSampleIorN(ctx.GetHeroLambda());
#endif
    return -1;
}

void Dielectric_Polarized(float n1, float n2, float cosTi, out float rp2, out float rs2)
{
    float sin2Ti = 1.0f - cosTi * cosTi;

    float eta = n1 / n2;
    float eta2 = eta * eta;

    float sin2Tt = eta2 * sin2Ti;
    float cosTt = sqrt(max(0.0f, 1 - sin2Tt));

    if (sin2Tt >= 1.0f)
    {
        rp2 = rs2 = 1.0f;
        return;
    }

    float rp = (n1*cosTt - n2*cosTi) / max(1e-6f, n1*cosTt + n2*cosTi);
    rp2 = rp * rp;

    float rs = (n1*cosTi - n2*cosTt) / max(1e-6f, n1*cosTi + n2*cosTt);
    rs2 = rs * rs;
}

float Dielectric_Unpolarized(float n1, float n2, float cosTheta)
{
    cosTheta = saturate(cosTheta);

    float rp2, rs2;
    Dielectric_Polarized(n1, n2, cosTheta, rp2, rs2);

    float reflectanceProb = (rp2 + rs2) * 0.5f;
    return reflectanceProb;
}

#endif