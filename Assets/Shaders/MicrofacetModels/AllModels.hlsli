#ifndef H_ALL_MODELS_H
#define H_ALL_MODELS_H

#include "MicrofacetModels/MicrofacetUtils.hlsli"
#include "MicrofacetModels/ResolveMM.hlsli"

// TODO:

//  #if defined(NDF_TYPE_GGX)
//  #    ifdef ANISOTROPY_ENABLED
//          float alpha = RoughnessToAlpha_GGX(roughness);
//  float2 alphaXY = AlphaToAnisoAlpha(alpha, anisoDirAndStrength.z);
//  float alphaX = max(1e-3, alphaXY.x);
//  float alphaY = max(1e-3, alphaXY.y);
//  bool isAniso = abs(alphaX - alphaY) > 0.0001f;
//  float3 H_s;
//  if (isAniso)
//  {
//      float3 anisoT = float3(anisoDirAndStrength.xy, 0);
//      float3 anisoB = float3(-anisoDirAndStrength.y, anisoDirAndStrength.x, 0);
//      float3 anisoN = float3(0, 0, 1);
//      H_s = SampleH_GGXAniso(alphaX, alphaY, anisoDirAndStrength.xy, anisoT, anisoB, anisoN, u1, u2);
//  }
//  else
//      H_s = SampleH_GGX(alpha * alpha, u1, u2);
//  #    else
//  #        ifdef SAMPLE_VISIBLE_NORMALS
//  float alpha = RoughnessToAlpha_GGX(roughness);
//  float a2 = max(1e-6f, alpha * alpha);
//  float u3 = Rand01_Bounce(DIM_D_BSDF_U3, rngInfo);
//  float3 H_s = SampleH_VCavity_VNDF(a2, V_s, u1, u2, u3);
//  #        else
//  MicrofacetModel ggxSmithIsoTest;
//  ggxSmithIsoTest.Init(roughness);
//  float3 H_s = ggxSmithIsoTest.Sample(u1, u2);
//
//  float alpha = RoughnessToAlpha_GGX(roughness);
//  float a2 = max(1e-6f, alpha * alpha);
//  //float3 H_s = SampleH_GGX(a2, u1, u2);
//  #        endif
//  #    endif
//  #elif defined(NDF_TYPE_BECKMANN) // TODO
//  float alpha = RoughnessToAlpha_Beckmann(roughness); // TODO: Walters Trick here?
//  float a2 = max(1e-6f, alpha * alpha);
//  float3 H_s = SampleH_Beckmann(a2, u1, u2);
//  #endif

//  #if defined(NDF_TYPE_GGX)
//  #    ifdef ANISOTROPY_ENABLED
//          if (isAniso)
//          {
//              D = D_GGXAniso(H_s, alphaX, alphaY);
//              //float G = G_SmithGGXAniso(L_s, V_s, alphaX, alphaY, anisoT, anisoB, anisoN);
//              G = G_GGXAniso(L_s, V_s, alphaX, alphaY);
//              pdf = Pdf_GGXAniso(D, NdH, VdH);
//          }
//          else
//          {
//              D = D_GGX(NdH, alpha * alpha);
//              G = G_SmithGGX(NdL, NdV, alpha * alpha);
//              pdf = Pdf_GGX(D, NdH, VdH);
//          }
//  #    else
//  #        ifdef SAMPLE_VISIBLE_NORMALS
//          D = D_GGX(NdH, a2); // Wrong
//          G = G_VCavity(V_s, L_s, H_s);
//          float Dv = D * G1_VCavity(NdV, a2) * max(0.0f, VdH) / NdV;
//          pdf = Pdf_GGX_VNDF(Dv, VdH);
//  #        else
//          D = D_GGX(NdH, a2);
//          G = G_SmithGGX(NdL, NdV, a2);
//          pdf = Pdf_GGX(D, NdH, VdH);
//  #        endif
//  #    endif
//  #elif defined(NDF_TYPE_BECKMANN) // TODO
//          D = D_Beckmann(H_s, a2);
//          G = G_Beckmann(V_s, L_s, alpha);
//          //pdf = Pdf_General(D, G1_Beckmann(V_s, alpha), V_s, H_s, pdfSampleVisibleArea);
//  #endif

#endif