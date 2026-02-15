## RMSE Tester

GUI Button: Take snapshot into slot {A,B}
- Copies the inputRTV into a slot {A,B} buffer

GUI Button: Compute RMSE between slots
- Runs RMSE CS on the two slots
- Readback RMSE and show on GUI

GUI InputInt: Golden Frame Snapshot (N)
GUI InputText: Golden File Path (Relative to Data/GoldenImages/)
GUI Button: Compute Golden Image
- Runs the path-tracer for N frames and then puts a snapshot into slot A. Then readsback the buffer and saves it into a file

GUI InputInt: Convergence Test Max Frames
GUI InputInt: Frame Bin Size
GUI InputText: Test Name
GUI Button: Run Convergence Test
- Loads golden image into slot A if not already there
- for (int i = 0; i < TestMaxFrames; i += FrameBinSize)
-   Set PT max frames to i
-   Wait for PT to pause
-   Copy into slot B
-   Run RMSE CS
-   Readback RMSE and write it alongside i to Data/RMSEs/TestName.txt
- Run python and give TestName as argument
- Python copies RMSEs from file and puts into a vector
- Plot data with Bokeh

Stretch Goal: FLIP model

## Spectral Tracing

Surface/Volume interactions stay mostly the same, but you use wavelengths instead of colors
First test: Do a color roundtrip, reconstruct an srgb image using random samples in the spectral domain. Then convert to CIE and srgb. When the error is less than 1 ppm then it's good.
Requires rewriting the shaders
Hero model doesn't work with transmission (Glass, clouds, etc). This is because it assumes scatttering has no wavelength dependency 

Put all spectral notes into a new file, this might get large

See:
https://github.com/ashpil/moonshine 