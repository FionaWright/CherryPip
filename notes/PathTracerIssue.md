# Path Tracer Issue

## Symptoms

**PT doesn't average/accumulate/converge properly across frames**

![alt text](image.png)  
fig1. 1000 Frames, 1SPP, poor convergence

![alt text](image-2.png)  
fig2. 100,000 Frames, 1SPP, little change compared to fig1

**PT seems to average/converge much better through SPP per-frame**  
**SPP is inversly proportional to brightness**

![alt text](image-3.png)  
fig3. 1000 Frames, 20SPP, better convergence but darker overall

**RNG seems ok. Did analysis on readback data and it shows correct std dev**

![alt text](image-1.png)  
fig4. 300,000 Samples of readback data from my RNG function (0-255 range due to Rgba8Unorm)

![alt text](image-5.png)  
![alt text](image-6.png)  
fig5, fig6. 1 Frame vs 1000 Frames of accumulated First Bounce Direction. Looks good, accumulates well!

![alt text](image-9.png)  
fig7. Normals for comparison, little darker I guess? 

**Furnace Test looks good**

![alt text](image-4.png)  
fig7. Perfect furnace test result

**First bounce Hit Distance doesn't accumulate well**

![alt text](image-7.png)  
![alt text](image-8.png)  
fig8, fig9. Distance from first bounce to second hit. 1 Frame vs 300 Frames. Brightness continues to increase as frames increases, not averaging properly

## Facts

- Single and only light source, emission strength 15
- Any miss returns black
- Cosine-weighted hemisphere direction `normalize(normal + RandSphere())`
- Using PCG random function