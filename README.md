# UBC
Until Being Crowned

## Build Instructions (Windows)

### Prerequisites
1. **Git**: Download and install Git.
    *   *Required for vcpkg to download libraries.*
2.  **Configure the project with CMake.**
    *   You **must** point CMake to the vcpkg toolchain file.
    *   We add `-DVCPKG_TARGET_TRIPLET=x64-windows` to ensure 64-bit libraries are built (matches VS default).
    *   *Note: The first time you run this, vcpkg will automatically download and build SFML 2.6.1 and TGUI defined in `vcpkg.json`. This may take a few minutes.*
    *   Replace the example path with the absolute path to your vcpkg installation folder.
    *   **Important**: Use forward slashes (`/`) for the path.

    ```bash
    # Example command using your path:
    cmake -B build -A x64 -S . -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
    ```
3.  **Compiler**: Visual Studio (MSVC) or MinGW-w64.
4.  **vcpkg** (Recommended for managing dependencies):
    *   **Option A (Command Line)**:
        *   Clone vcpkg: `git clone https://github.com/microsoft/vcpkg`
    *   **Option B (Manual Download)**:
        *   Download the vcpkg ZIP file and extract it.
    *   **Bootstrap**: Open a terminal in the vcpkg folder and run `.\bootstrap-vcpkg.bat`.

### Generate project
``` bash
cmake -B build -A x64 -S . -DCMAKE_TOOLCHAIN_FILE=C:\Users\Stanislas\Documents\GitHub\vcpkg\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build
```