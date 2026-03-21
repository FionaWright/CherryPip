#ifndef H_CIE_2006_LOGISTIC_H
#define H_CIE_2006_LOGISTIC_H

// https://en.wikipedia.org/wiki/Logistic_function

float Logistic(float x, float L, float k, float x0)
{
    return L / (1.0f + exp(-k * (x - x0)));
}

/*

Red:
x<500 = f(x, 1/3, -0.2, 400)
x<610 = f(x, 1, 0.5, 587)
x>610 = 1/3 + f(x, 2/3, -0.1, 670)

Green:
x<450 = f(x, 0.35, -0.2, 410)
x<525 = f(x, 1, 0.5, 478)
x<610 = f(x, 1, -0.5, 587)
x>610 = f(x, 1/3, 0.1, 665)

Blue:
x<460 = 1/3 + f(x, 2/3, 0.16, 400)
x<550 = f(x, 1, -0.5, 478)
x>550 = f(x, 1/3, 0.1, 670)

*/

#define ONE_THIRD 0.3333333f
#define TWO_THIRD 0.6666666f

float CIEBasis_Logistic_X(float lambda)
{
    return lambda < 500 ? Logistic(lambda, ONE_THIRD, -0.2f, 400) :
           lambda < 610 ? Logistic(lambda, 1, 0.5f, 587) :
                          ONE_THIRD + Logistic(lambda, TWO_THIRD, -0.1f, 670);
}

float CIEBasis_Logistic_Y(float lambda)
{
    return lambda < 450 ? Logistic(lambda, 0.35f, -0.2f, 410) :
           lambda < 525 ? Logistic(lambda, 1, 0.5f, 478) :
           lambda < 610 ? Logistic(lambda, 1, -0.5f, 587) :
                          Logistic(lambda, ONE_THIRD, 0.1f, 665);
}

float CIEBasis_Logistic_Z(float lambda)
{
    return lambda < 460 ? ONE_THIRD + Logistic(lambda, TWO_THIRD, 0.16f, 400) :
           lambda < 550 ? Logistic(lambda, 1, -0.5f, 478) :
                          Logistic(lambda, ONE_THIRD, 0.1f, 670);
}

#endif