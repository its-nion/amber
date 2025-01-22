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
- [Developement](#developement)
- [Usage](#usage)
- [Credits](#credits)

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

<!-- Developement -->
## Developement
Amber is a simple C++ renderer that generates images in real-time with vulkan through a compute shader pipeline. It generates pseudo-random noise, transforms it into FBM-noise, and uses warping and adjustments to create visually appealing outputs. These can be saved in PNG format, with variables adjusted dynamically via Push-Constants.


<br />

<!-- Usage -->
## Usage
Amber is currently only supported on windows. Follow these steps to run it:
1. Go to the [latest release page](https://github.com/its-nion/amber/releases/latest)
2. Download ``Amber.exe``
3. Run ``Amber.exe``

<br />

<!-- Credits -->
## Credits
Special thanks to the individuals below, whose articles served as inspiration and played a key role in making this project a reality
- **Victor Blanco** with his [Vulkan Guide](https://vkguide.dev/)
- **Patricio Gonzalez Vivo & Jen Lowe** with their article about [Fractal Brownian Motion](https://thebookofshaders.com/13/)
- **Inigo Quilez** with his article about [Domain Warping](https://iquilezles.org/articles/warp/)
