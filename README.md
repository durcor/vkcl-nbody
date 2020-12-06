# vkcl
A library written in C++ that is focused on making compute with Vulkan a lot less painless with performance in mind.

## Building
### Library Requirements:
- [Meson](http://mesonbuild.com/) Version 0.47 or higher
- Any C++ Compiler
	- gcc
	- clang

### Test Requirements:
- [glslang](https://github.com/KhronosGroup/glslang) for compiling SPIR-V Shaders

### Compilation
To build only vkcl, you can run:
```sh
sh build.sh vkcl
```

If you desire to build the n-body simulator test as well, you can run:
```sh
sh build.sh nbody
# or also simply
sh build.sh
```

### Build Options:
The following build options are available:
* enable_debug - Enables debugging symbols and Vulkan validation layers.
* enable_test - Compiles the test program, which can then be run with `meson test`.

These build options are off by default, but can be enabled with:
```sh
meson configure build -Denable_debug=true -Denable_test=true
```

## Tony's N-Body Test

### Some N-Body Calculation Benchmarks
- 5700XT w/ RADV/LLVM - 18.18 FPS
- 5700XT w/ RADV/ACO - 17.54 FPS
- 5700XT w/ AMDVLK - 11.36 FPS
- 5700XT w/ AMDVLK-PRO - 2.35 FPS
- 2060S - ~2 FPS

## License
[Simplified BSD License](LICENSE)
