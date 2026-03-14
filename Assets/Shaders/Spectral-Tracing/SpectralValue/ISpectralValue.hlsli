// Interface for SpectralValue super-class

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

void FromRGB(float3 rgb, SpectrumType type, float lambda);
void AddRGB(float3 rgb, SpectrumType type, float lambda);
float3 ToRGB(float lambda);