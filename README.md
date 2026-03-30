# webro

JUCE-based audio plugin project prepared for development in VS Code via CMake.

## Requirements

- Full Xcode app on macOS for plugin toolchains and SDKs
- Xcode command line tools
- [CMake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)
- A local JUCE checkout at `../JUCE` relative to this project, or a custom `JUCE_DIR`

## Open in VS Code

1. Open `/Users/kk/Desktop/YO/code/webro` in VS Code.
2. Install the recommended extensions if VS Code suggests them.
3. Run the `CMake: configure` task once, or let CMake Tools configure on open.
4. Build with the `CMake: build Debug` task.

## Manual build

```bash
cmake -S . -B build -G Ninja -DJUCE_DIR=../JUCE
cmake --build build --config Debug
```

The generated plugin artefacts are copied into the project-local `artifacts` directory:

- `artifacts/VST3/webro.vst3`

JUCE still uses the internal `build/..._artefacts` directories during compilation, but the final developer-facing output is copied into `artifacts`.

The build also injects the local `cmake/shims` directory into `CPATH` automatically, so no manual environment export is required before running CMake.

## Current machine status

This project also includes a local compatibility shim for `Foundation/NSGarbageCollector.h`, because the macOS 26.2 SDK bundled with Xcode 26.3 references that header from `Foundation.h` but does not ship the file itself. The shim is only there to unblock JUCE/CMake builds on that SDK.
