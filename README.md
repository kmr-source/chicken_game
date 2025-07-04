
---

```markdown
# 🐔 Chicken

A cross-platform C++14 graphics application using OpenGL, GLFW, SDL2, and GLM.

## Features

- OpenGL-based rendering
- SDL2 audio support (via SDL2_mixer)
- Cross-platform build support: **Windows, Linux, and macOS**
- Clean CMake-based build system
- Auto-injected shader/project path via `configure_file`

---

## 📁 Project Structure

```

chicken/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── ...
├── ext/
│   ├── stb\_image/
│   ├── gl3w/
│   ├── glm/
│   ├── sdl/
│   └── glfw/

````

---

## 🔧 Build Instructions

### 🐧 Linux

**Dependencies**: `cmake`, `pkg-config`, `GLFW`, `SDL2`, `SDL2_mixer`, `OpenGL`, `GLM`

```bash
sudo apt update
sudo apt install cmake pkg-config libglfw3-dev libsdl2-dev libsdl2-mixer-dev libglm-dev
mkdir build
cd build
cmake ..
make
````

---

### 🍎 macOS

Yes, **this project supports macOS** — but you'll need to install the required libraries manually using **Homebrew**.

#### ✅ Prerequisites (Install Homebrew)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### ✅ Install dependencies:

```bash
brew install cmake pkg-config glfw sdl2 sdl2_mixer glm
```

#### ✅ Build:

```bash
mkdir build
cd build
cmake ..
make
```

#### ⚠️ Notes for macOS:

* OpenGL is deprecated on macOS, but still available and functional.
* If your Mac has an M1/M2 chip, the max supported OpenGL version is 4.1 natively.
* The build system links required Apple frameworks like `Cocoa` and `CoreFoundation`.
* If you encounter errors related to OpenGL or `gl3w`, make sure your context and loader setup is macOS-compatible.

---

### 🪟 Windows

Uses precompiled binaries in `ext/`. You must have:

* Visual Studio (2022 recommended)
* CMake in your `PATH`

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

---

## 🖼️ Running the Application

After building:

* On **Linux/macOS**:
  Run the binary:

  ```bash
  ./build/chicken
  ```

* On **Windows**:
  The required `.dll` files are automatically copied to the build folder:

  ```bash
  .\build\Release\chicken.exe
  ```

---

## ⚙️ Configuration

The file `ext/project_path.hpp` is generated at configure-time using CMake's `configure_file()`.

You can edit `ext/project_path.hpp.in` to change how paths are resolved inside your app (e.g., to locate shaders or resources).

---

## 🧪 Development Tips

* Enable verbose output:

  ```bash
  cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
  ```
* To manually control source files instead of using `file(GLOB ...)`, uncomment the `set(SOURCE_FILES ...)` section in `CMakeLists.txt`.

---

