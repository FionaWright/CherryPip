// Interface for SpectralValue base class

void InitBlack();
void InitWhite();

void Add(SpectralValue v);
void Add(float scalar);
void Sub(SpectralValue v);
void Sub(float scalar);
void Mul(SpectralValue v);
void Mul(float scalar);
void Div(SpectralValue v);
void Div(float scalar);

void Sqrt();
void Clamp(float lo, float hi);
void Normalize();
void Exp();

float Max();

void FromRGB(float3 rgb, SpectrumType type, SpectralContext ctx);
void AddRGB(float3 rgb, SpectrumType type, SpectralContext ctx);
float3 ToRGB(SpectralContext ctx);

bool IsBlack();