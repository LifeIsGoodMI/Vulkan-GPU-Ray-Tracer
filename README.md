# Vulkan-GPU-Ray-Tracer

A basic realtime GPU Ray Tracer (**only direct lighting**) based on the Vulkan graphics & compute API.
Since this is my first experience with a graphics API, I've oriented my codebase on Alexander Overvoorde's Vulkan API tutorial (see: https://vulkan-tutorial.com/)

The ray tracer is completly implemented using a compute shader. Most of the C++ stuff is around creating & handling the Vulkan instance as well as passing geometry & material information to the compute shader.

Before you're running the solution, make sure to link LunarG's Vulkan SDK as well as GLFW to the project since it is used to display the resulting image in realtime.
When you're running the precompiled binary, make sure that the 'shader' folder is located in the same directory as the binary.

**Note: I've tested the code only on Windows, it might not run correctly on any other operating system.**
