<!-- HEADER -->
<div align="center">
  <a href="https://github.com/its-nion/Amber">
    <img src="icon/Amber.png" alt="Logo" width="100" height="100">
  </a>

  <h3 align="center">Amber</h3>
  
  <p align="center">
    A simple way to generate beautiful images
    <br />
    <br />
    <a href="https://github.com/its-nion/amber/releases/latest">Latest Build</a>
    ·
    <a href="https://github.com/its-nion/Amber/issues/new?assignees=&labels=bug&projects=&template=bug_report.md&title=">Report Bug</a>
  </p>
</div>

<br />

<!-- TOC -->
## Table of Contents
- [About](#about)
- [Features](#features)
- [Sample Images](#sample-images)
- [Used Technologies](#used-technologies)
- [How It Works](#how-it-works)
- [Running Amber](#running-amber)
- [Building Amber](#building-amber)
- [Future Improvements](#future-improvements)
- [Acknowledgments](#acknowledgments)

<br />

<!-- ABOUT -->
## About
Amber is a lightweight image generator that lets you design stunning patterns in real-time and export them as high-quality PNGs. It’s a quick and easy way to create beautiful visuals.

<div style="display: flex", align="center">
  <img src="images/about-1.gif" alt="about-gif" style="width: 100%">
</div>

<br />

<!-- FEATURES -->
## Features
- **Lightweight** · No installation or large files required
- **Customizable** · Full control over every design element
- **High Quality** · Images are drawn directly in your preferred resolution
- **Free** · No cost whatsoever!

<br />

<!-- SAMPLE IMAGES -->
## Sample Images
<div style="display: flex", align="center">
  <img src="images/red-1.png" alt="red image" style="width: 49%">
  <img src="images/white-1.png" alt="white image" style="width: 49%">
  <img src="images/desert-1.png" alt="desert image" style="width: 99%">
  <img src="images/blue-1.png" alt="water image" style="width: 49%">
  <img src="images/blue-2.png" alt="cloud image" style="width: 49%">
</div>

<br />

<!-- Used Technologies -->
## Used Technologies
Amber utilizes the following libraries and frameworks:
- Vulkan SDK - Low-level graphics API for high-performance rendering
- GLFW - Window and input handling
- GLM - Mathematics library for vector and matrix operations
- ImGui - Simple user interface library
- stb_image - Image creation and export
- VkBootstrap - Simplified Vulkan setup
- Vulkan Memory Allocator (VMA) - Efficient memory management for Vulkan
- Bin2cpp - C++ embedding library


<br />

<!-- How It Works -->
## How It Works
Amber generates images using a Vulkan-based compute shader pipeline. At startup, it initializes Vulkan, sets up a swapchain for rendering, and configures descriptor sets and synchronization mechanisms. The application then builds a dedicated compute pipeline to process image data in parallel on the GPU. Users can interact with the ImGui interface to modify rendering parameters in real time through push-constants.

The compute shader executes a warped Fractal Brownian Motion (FBM) algorithm, which layers multiple octaves of procedural noise. By applying a nonlinear warping function, the shader distorts the noise pattern, producing complex organic textures. This enables users to create a wide variety of abstract visuals with smooth, natural variations.

<br />

<!-- Running Amber -->
## Running Amber
Amber is currently only supported on windows. Follow these steps to run it:

1. Go to the [latest release page](https://github.com/its-nion/amber/releases/latest)
2. Download and run ``Amber.exe``
3. Customize your image via the user interface on the right
4. Hover over "File" and press "Export" to save your image

<br />

> [!Warning]  
> The executable file got flagged as a virus on my pc, probably because it's a singular small file. If that happens to you aswell, you can either quarantine the file from the windows defender firewall or build the file yourself, which is explained in the next step.

<br />

## Building Amber
1. Clone the repository:
```
git clone https://github.com/its-nion/amber.git
```
2. Go into the "visual-studio" folder and open "Amber.sln" with Visual Studio
3. Build/Run the project

<br />

> [!Note]  
> Before building the project, you need to have the Vulkan SDK installed. All other libraries should be included in the "third-party" folder, but make sure that all dependencies are linked correctly.

<br />

<!-- Future Improvements -->
## Future Improvements
- Expand shader functionality for additional visual effects
- Clean up project structure
- Additional export formats

<br />

<!-- Acknowledgments -->
## Acknowledgments
Special thanks to the individuals below, whose articles served as inspiration and played a key role in making this project a reality
- **Victor Blanco** with his [Vulkan Guide](https://vkguide.dev/)
- **Patricio Gonzalez Vivo & Jen Lowe** with their article about [Fractal Brownian Motion](https://thebookofshaders.com/13/)
- **Inigo Quilez** with his article about [Domain Warping](https://iquilezles.org/articles/warp/)
