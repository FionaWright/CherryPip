#ifndef H_HERO_SAMPLE_H
#define H_HERO_SAMPLE_H

#define NUM_HERO_SAMPLES 3

struct HeroSample
{
    void Init(float values[NUM_HERO_SAMPLES]);

    float HeroValue;
    float NonHeroValues[NUM_HERO_SAMPLES-1];
};

void HeroSample::Init(float values[NUM_HERO_SAMPLES])
{
    HeroValue = values[0];
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES-1; i++)
        NonHeroValues[i] = values[i+1];
}

#endif