This project is in its early days. As of writing this I've got a basic framework for a simple D3D12 engine. It can render using a raster pipeline already.  

The project structure is much improved from my previous engine. I'm using CMAKE now and the source/build are completely seperated as is the engine/app. I've begun tracking the FPS from the beginning and once I get path-tracing up and running I might make some CI/CD-esque tests to confirm I don't lose perf throughout development.  

My next task is to begin using Ray Query and BVHs to render a scene. I will be setting up a lot of debugging and profiling tools to help me in the future  

:D
