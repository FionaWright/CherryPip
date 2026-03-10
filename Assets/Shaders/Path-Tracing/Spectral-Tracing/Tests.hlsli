
#define SPECTRAL_TEST_ROUND_TRIP_2

#ifdef SPECTRAL_TEST_CIE

// CIE Test
Spectrum white = WhiteSpectrum_D65();
float lambda = (float)VISIBLE_LIGHT_SPECTRUM_MIN + (saturate(input.uv.x) * (float)VISIBLE_LIGHT_SPECTRUM_SIZE);
float3 cie = float3(SampleCIE(lambda, cCIE_X), SampleCIE(lambda, cCIE_Y), SampleCIE(lambda, cCIE_Z));
return float4(cie, 1);

#endif

//============================================

#ifdef SPECTRAL_TEST_SPECTRUM_TO_RGB

// Spectrum to RGB Test
Spectrum s = BlackSpectrum();
for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
{
    float lambda = VISIBLE_LIGHT_SPECTRUM_MIN + (i * SPECTRUM_DELTA_LAMBDA);
    bool inRange = lambda >= 380.0f && lambda <= 450.0f;
    s.Samples[i] = inRange ? 1.0f : 0.0f;
}
float3 r = SpectrumToRGB(s);
return float4(r, 1);

#endif

//============================================

#ifdef SPECTRAL_TEST_SPECTRUM_TO_RGB_2

// Spectrum to RGB Test 2
Spectrum s = WhiteSpectrum_D65();
s.Mul(input.uv.y);
float3 r;
if (input.uv.x < 0.5f)
    r = input.uv.y;
else
	r = SpectrumToRGB(s);
return float4(r, 1);

#endif

//============================================

#ifdef SPECTRAL_TEST_SPECTRUM_TO_RGB_3

// Spectrum To RGB Test 3
Spectrum s = RedSpectrum();
s.Mul(input.uv.y);
float3 r;
if (input.uv.x < 0.5f)
    r = input.uv.y * float3(1,0,0);
else
	r = SpectrumToRGB(s);
return float4(r, 1);

#endif

//============================================

#ifdef SPECTRAL_TEST_ROUND_TRIP

// Round-Trip Test
float3 color = float3(input.uv.yyy);
Spectrum s;
s.InitFromRGB(color, eReflectance);
float3 r = SpectrumToRGB(s);
if (input.uv.x < 0.5f)
    r = color;
if (r.x >= 1.0f)
    r = float3(1, 0, 1);
return float4(r, 1);

#endif

//============================================

#ifdef SPECTRAL_TEST_ROUND_TRIP_2

// Round-Trip Test 2
DebugWindow wRed(0, 0, 0.083, 1.0);
DebugWindow wRedRT(0.083, 0, 0.083*2, 1.0);
DebugWindow wGreen(0.083*2, 0, 0.083*3, 1.0);
DebugWindow wGreenRT(0.083*3, 0, 0.083*4, 1.0);
DebugWindow wWhite(0.083*4, 0, 0.083*5, 1.0);
DebugWindow wWhiteRT(0.083*5, 0, 0.083*6, 1.0);
DebugWindow wColors(0.5, 0, 1.0, 0.5);
DebugWindow wColorsRT(0.5, 0.5, 1.0, 1.0);
Spectrum s;
float3 r;
if (wRed.Select(input.uv))
    r = float3(1, 0, 0) * wRed.m_UV.y;
else if (wRedRT.Select(input.uv))
{
    s.InitFromRGB(float3(1, 0, 0)*wRedRT.m_UV.y, eReflectance);
    r = SpectrumToRGB(s);
}
else if (wGreen.Select(input.uv))
    r = float3(0, 1, 0) * wGreen.m_UV.y;
else if (wGreenRT.Select(input.uv))
{
    s.InitFromRGB(float3(0, 1, 0)*wGreenRT.m_UV.y, eReflectance);
    r = SpectrumToRGB(s);
}
else if (wWhite.Select(input.uv))
    r = float3(1, 1, 1) * wWhite.m_UV.y;
else if (wWhiteRT.Select(input.uv))
{
    s.InitFromRGB(float3(1, 1, 1)*wWhiteRT.m_UV.y, eReflectance);
    r = SpectrumToRGB(s);
}
else if (wColors.Select(input.uv))
{
    float lambda = lerp(380, 720, wColors.m_UV.x);
    r = WavelengthToRGB(lambda);
}
else if (wColorsRT.Select(input.uv))
{
    float lambda = lerp(380, 720, wColorsRT.m_UV.x);
    s.InitFromRGB(WavelengthToRGB(lambda), eReflectance);
    r = SpectrumToRGB(s);
}
return float4(r, 1);

#endif