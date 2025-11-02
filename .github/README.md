<h1 align="center">
  R-Engine<br>
  A C++23 Game Engine
  <br>
  <img src="https://raw.githubusercontent.com/catppuccin/catppuccin/main/assets/palette/macchiato.png" width="600px"/>
  <br>
</h1>

<div align="center">
  <p></p>
  <div align="center">
     <a href="https://github.com/Leorevoir/R-Engine/stargazers">
        <img src="https://img.shields.io/github/stars/Leorevoir/R-Engine?color=F5BDE6&labelColor=303446&style=for-the-badge&logo=starship&logoColor=F5BDE6">
     </a>
     <a href="https://github.com/Leorevoir/R-Engine/">
        <img src="https://img.shields.io/github/repo-size/Leorevoir/R-Engine?color=C6A0F6&labelColor=303446&style=for-the-badge&logo=github&logoColor=C6A0F6">
     </a>
     <a href="https://github.com/Leorevoir/R-Engine/blob/main/LICENSE">
        <img src="https://img.shields.io/static/v1.svg?style=for-the-badge&label=License&message=GPL3&colorA=313244&colorB=F5A97F&logo=unlicense&logoColor=F5A97F&"/>
     </a>
  </div>
  <br>
</div>

<p align="center">
  A data-oriented C++23 game engine built with Raylib, featuring a modern ECS architecture and Lua scripting.
</p>

## About The Project

R-Engine is a lightweight, cross-platform game engine designed for performance and flexibility. It is built upon a modern, data-oriented **Entity Component System (ECS)** architecture inspired by Bevy. This approach promotes clean code, performance, and modularity.

The engine uses **Raylib** for low-level graphics and hardware abstraction, and integrates a **Lua** scripting interface, allowing for rapid prototyping and flexible game logic.

### Key Features

- **Modern C++23:** Leverages the latest C++ features for performance and clean code.
- **Data-Oriented ECS:** A high-performance Entity Component System for modular and scalable game architecture.
- **Plugin-Based:** Extend the engine with custom functionality through a simple plugin interface.
- **Cross-Platform:** Supports Windows, macOS, and various Linux distributions.
- **Raylib Backend:** Built on the simple and powerful Raylib library.
- **Lua Scripting:** Integrated Lua support for flexible game logic.
- **Post-Processing:** Comes with a wide array of built-in post-processing shaders.
- **Simple Build System:** Uses CMake and provides easy-to-use build scripts for all platforms.

## Documentation

The engine is documented using Docusaurus. The documentation covers the core concepts of the ECS, advanced features, and provides a full API reference.

You can build and view the documentation locally:

```bash
cd docs/
npm install
npm start
```

## Getting Started

### Prerequisites

- A C++23 compatible compiler (GCC, Clang, or MSVC)
- CMake (>= 3.16)
- Ninja (optional, but recommended for faster builds)

A Windows installer is provided in `scripts/Windows` to help set up the required dependencies.

### Installation

Clone the repository and its submodules (Raylib and Lua) recursively:

```bash
git clone --recurse-submodules -j$(nproc) git@github.com:Leorevoir/R-Engine.git
cd R-Engine/
```

## Build and Run

The engine provides simple shell and PowerShell scripts for building the examples.

### Linux / macOS

```bash
# Build the examples in Release mode
./build.sh

# Build in Debug mode
./build.sh -d

# Run an example
./r-engine__bouncing_balls
```

### Windows

```powershell
# Build the examples in Release mode
.\build.ps1

# Build in Debug mode
.\build.ps1 -d

# Run an example
.\r-engine__bouncing_balls.exe
```

### Manual Build (CMake)

You can also build the project manually using CMake and Ninja:

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
cd ..
```

## Post-Processing Showcase

R-Engine includes a variety of built-in post-processing shaders ready to use:

- Grayscale
- Posterization
- Dream Vision
- Pixelizer
- Cross Hatching
- Cross Stitching
- Predator View
- Scanlines
- Fisheye
- Sobel
- Bloom
- Blur
- Contrast
- Protanopia, Deuteranopia, Tritanopia (Color blindness simulation)

## Development

Interested in contributing? We welcome pull requests and issues.

> TODO: Add `CODE_OF_CONDUCT.md` & `CONTRIBUTING.md`

## Tested On

- Windows
- macOS
- Debian
- Arch Linux
- Ubuntu
- Fedora

## License

> TODO (This project is licensed under the GNU GPLv3 License)
