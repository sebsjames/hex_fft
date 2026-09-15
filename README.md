# Hexagonal FFT

The Hexagonal FFT, following Nicholas I. Rummelt's PhD  thesis *Array set addressing: Enabling efficient hexagonally sampled image processing*, University of Florida, 2010.

## Dependencies

If you are using Debian or Ubuntu, the following `apt` command should
install the mathplot dependencies.

```bash
sudo apt install build-essential cmake git ninja-build  \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev clang-20 clang-tools-20
```

On Arch Linux the following command should install dependencies:
```bash
sudo pacman -S vtk lapack blas freeglut glfw-wayland
# Plus install clang20
```

On Fedora Linux, the following command should install the required dependencies
```bash
sudo dnf install cmake libglvnd-devel mesa-libGL-devel glfw-devel freetype-devel
# Plus install clang20
```

I'd love to know the equivalents for other Linux distributions so I
can include them in the mathplot documentation, so if you know,
please pull-request them!

If you're building on a Mac, you can refer to the [Mac
README](https://github.com/sebsjames/mathplot/blob/main/README.build.mac.md#installation-dependencies-for-mac)
for help. You only need to obtain and build
[glfw3](https://github.com/sebsjames/mathplot/blob/main/README.build.mac.md#glfw3);
OpenGL and Freetype should already be installed by default.

## Building

To build and run the example:

```bash
# Clone this example
git clone git@github.com:sebsjames/hex_fft

# Bring in the three submodules - sebsjames/mathplot, sebsjames/maths and nlohmann/json
# Your project will need these submodules too. They don't HAVE to be
# submodules, you can just copy or symlink the three codebases if you
# prefer.

cd hex_fft # or whatever you named your fork/copy
git submodule init
git submodule update

# Build prog1 in a 'build' directory
mkdir build
cd build
CC=clang-20 CXX=clang++-20 cmake .. -GNinja
ninja
./fft_play
```
