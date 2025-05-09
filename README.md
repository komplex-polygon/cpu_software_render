# CPU Software Renderer

This repository hosts a software renderer implemented entirely in C. It's designed to render 3D models and textures without dedicated GPU hardware, offering various output methods, including terminal-based visuals.

A portion of the codebase, particularly for complex mathematical operations or low-level utilities, was developed with the assistance of AI tools like Deepseek.

## Features

*   **Runtime Asset Loading:** Load OBJ models and PNG textures at runtime. You can modify media files without recompiling the program.
*   **Multiple Rendering Modes:**
    *   **Unicode Braille:** Renders scenes using Braille characters in the terminal (monochrome).
    *   **Unicode Half-Block:** Renders scenes using half-block Unicode characters in the terminal.
    *   **Framebuffer:** Direct pixel rendering to a graphical framebuffer (platform-dependent).
*   **OBJ & PNG Support:** Natively supports Wavefront OBJ (`.obj`) files for 3D models and Portable Network Graphics (`.png`) for textures.
*   **Low-Latency Controls (Linux):** Utilizes direct reads from `/dev/input/` on Linux for raw keyboard data, enabling responsive input. This feature typically requires root privileges.
*   **Customizable Startup Flags:** Control various settings like rendering mode, camera behavior, and initial zoom level via command-line arguments.

## Command-Line Flags

*   `-mode <mode_name>`: Sets the rendering mode.
    *   `mono`: Uses Unicode Braille characters (monochrome).
    *   `color`: Uses Unicode half-block characters.
    *   `buffer`: Renders to a direct framebuffer.
*   `-no-root`: Disables reading from `/dev/input/`. Use this if you are not running the program as root or are accessing it via SSH (where direct device access might be problematic or unwanted).
*   `-spin`: Enables automatic camera rotation around the loaded model. Can be used in conjunction with `-no-root`.
*   `-zoom <factor>`: Sets the initial zoom level as a floating-point number (e.g., `-zoom 1.5`). Can also be used with `-no-root`.

## Building

**Prerequisites:**
*   A C compiler (GCC is tested and recommended).
*   Standard C libraries, including the math library (`libm`).

**Compilation Command:**

Navigate to the root directory of the repository and execute the following command:

```bash
gcc main.c libs/lodepng.c -lm -O3 -o exec
