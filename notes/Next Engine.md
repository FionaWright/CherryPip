# Improvements for the next engine

## Structure

- Use DirectXTex instead of spng/tinyddsloader/etc

- Improve the GLTF loading, it's extremely messy as is. Reintroduce async loading but make sure it works this time

- Have scenes use proper array data structures instead of object-oriented smart pointers

- Improve the material / descriptor / heap systems, they're clunky and probably slow. Did I abstract too much?

- Solidify the separation between the Apps and Engine, it's still a little bit of a blurry line. Put them in separate repos next time? Have shaders local to the App?

- Combine RGB and Spectral renderers into one framework

- Use templates for better HLSL polymorphism 

Note: Many of the biggest issues are at the HWI level and/or from code I stole from my last engine instead of rewriting it

## Ambitions

- Have a cross-API HWI (+Vulkan / +Metal)

- USD Scene Loader

- Python Bindings

- Shaders compile at build time without sacrificing hot reloading

- Better GUI that is more game-engine-like. Entity-Component system with hierarchies and editing/saving/loading/etc

- C++ compilable shaders