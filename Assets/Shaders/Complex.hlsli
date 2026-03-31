#ifndef H_COMPLEX_H
#define H_COMPLEX_H

// Represents a complex number (a + bi) where i^2 == -1

struct Complex
{
    void Init(float re, float im);

    void Add(Complex c);
    void Sub(Complex c);
    void Mul(Complex c);
    void Mul(float x);
    void Div(Complex c);

    void Sqrt();
    void Neg();

    float Abs();

    float Re;
    float Im;
};

Complex CreateComplex(float re, float im)
{
    Complex c;
    c.Init(re, im);
    return c;
}

void Complex::Init(float re, float im)
{
    Re = re;
    Im = im;
}

void Complex::Add(Complex c)
{
    Re += c.Re;
    Im += c.Im;
}

Complex Add(Complex c1, Complex c2)
{
    Complex c = c1;
    c.Add(c2);
    return c;
}

void Complex::Sub(Complex c)
{
    Re -= c.Re;
    Im -= c.Im;
}

Complex Sub(Complex c1, Complex c2)
{
    Complex c = c1;
    c.Sub(c2);
    return c;
}

void Complex::Mul(Complex c)
{
    float a = Re;
    float b = Im;

    Re = a * c.Re - b * c.Im;
    Im = a * c.Im + b * c.Re;
}

void Complex::Mul(float x)
{
    Re *= x;
    Im *= x;
}

Complex Mul(Complex c1, Complex c2)
{
    Complex c = c1;
    c.Mul(c2);
    return c;
}

Complex Mul(Complex c1, float x)
{
    Complex c = c1;
    c.Mul(x);
    return c;
}

void Complex::Div(Complex c)
{
    float a = Re;
    float b = Im;

    float k = c.Re * c.Re + c.Im * c.Im;
    Re = (a * c.Re + b * c.Im) / max(1e-6, k);
    Im = (c.Re * b - a * c.Im) / max(1e-6, k);
}

Complex Div(Complex c1, Complex c2)
{
    Complex c = c1;
    c.Div(c2);
    return c;
}

// From std::sqrt
void Complex::Sqrt()
{
    if (Re == 0.0f && Im == 0.0f)
        return;

    float theta = atan2(Im, Re);
    float powerTerm = pow(Re * Re + Im * Im, 0.25f);
    Re = powerTerm * cos(theta);
    Im = powerTerm * sin(theta);

    if (Re < 0.0f)
        Neg();
}

Complex Sqrt(Complex c1)
{
    Complex c = c1;
    c.Sqrt();
    return c;
}

void Complex::Neg()
{
    Re = -Re;
    Im = -Im;
}

Complex Neg(Complex c1)
{
    Complex c = c1;
    c.Neg();
    return c;
}

float Complex::Abs()
{
    return sqrt(Re * Re + Im * Im);
}

float Abs(Complex c)
{
    return c.Abs();
}

Complex Ternary(bool cond, Complex a, Complex b)
{
    Complex r;
    r.Re = cond ? a.Re : b.Re;
    r.Im = cond ? a.Im : b.Im;
    return r;
}

#endif