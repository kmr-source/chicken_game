```markdown
# 🐔 Chicken

A cross-platform C++14 graphics application using OpenGL, GLFW, SDL2, and GLM.

## Features

- OpenGL rendering
- Audio support via SDL2_mixer
- Cross-platform build support (Windows, macOS, Linux)
- Clean CMake integration
- Optional shader path generation

---

## 🔧 Build Instructions

### 🐧 Linux / 🍎 macOS

> Dependencies required: `cmake`, `pkg-config`, `GLFW`, `SDL2`, `SDL2_mixer`, `OpenGL`

Install dependencies (example for Ubuntu/Debian):

```bash
sudo apt update
sudo apt install cmake libglfw3-dev libsdl2-dev libsdl2-mixer-dev libglm-dev
````

Then build:

```bash
mkdir build
cd build
cmake ..
make
```

### 🪟 Windows

1. Install [CMake](https://cmake.org/), [Visual Studio](https://visualstudio.microsoft.com/), and ensure CMake is in your `PATH`.
2. Clone the project and open a terminal in the root directory.
3. Build:

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" # or appropriate generator
cmake --build . --config Release
```

> Note: Precompiled libraries and DLLs for SDL2, SDL2\_mixer, and GLFW are expected in `ext/sdl/` and `ext/glfw/`.

---

## 🖼️ Running the Application

After building, the executable will be in:

* `build/chicken` (Linux/macOS)
* `build/Release/chicken.exe` (Windows)

On Windows, required DLLs will be copied automatically to the output directory.

---

## ⚙️ Configuration

### Shader Path Injection

`ext/project_path.hpp` is auto-generated during the CMake process. You can edit `ext/project_path.hpp.in` to customize.

---

## 🧪 Development Tips

* You can enable verbose CMake output with:

  ```bash
  cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
  ```
* If adding new `.cpp`/`.hpp` files, either:

  * Rely on the `file(GLOB ...)` in `CMakeLists.txt`, or
  * Manually add them to the `SOURCE_FILES` list (recommended for long-term maintenance).
