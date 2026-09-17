# Hexagonal FFT

The Hexagonal FFT, following Nicholas I. Rummelt's PhD  thesis *Array set addressing: Enabling efficient hexagonally sampled image processing*, University of Florida, 2010.

![A screenshot from the fft_example program showing an hexagonal image of a bicycle and its hexagonal FFT](https://github.com/sebsjames/hex_fft/blob/main/fft_example.png?raw=true)

The FFT implementation presented here can be found in [sebsjames/maths](http://github.com/sebsjames/maths); this repository provides a graphical example of its use. The implementation makes it possible to obtain the hexagonal FFT of an arbitrarily shaped [sm::hexgrid](https://github.com/sebsjames/maths/blob/main/sm/hexgrid.cppm). Your arbitrary hexgrid is placed inside a regular, rectangular hexgrid, with additional hex elements zero-padded.

## Example programs

There are four examples, fft_example, fft_static, fft_dynamic and fft_debug. Start with fft_example, which shows a bicycle image and its FFT (see 'Building', below). fft_static shows the FFTs of several functions (angled sine waves, mostly) and is helpful for getting some understanding of the FFT output. fft_dynamic demonstrates an animated function and FFT. fft_debug shows additional, internal stages in the many transforms that combine to make the hex FFT. This was useful during development and is left here to track down any future bugs.

![A screenshot from the fft_static program showing various 2D functions and their hexagonal FFTs](https://github.com/sebsjames/hex_fft/blob/main/fft_static.png?raw=true)
*The fft_static program allows you to see the FFT of several 2D functions*


## Using `sm::hexfft`

Create a hexgrid. The hexgrid constructor args are hex-hex distance, grid width and grid 'z' value (usually set to 0).

```c++
import sm.hexgrid;

sm::hexgrid<float> hg(0.01f, 4.0f, 0.0f);
hg.set_circular_boundary (1.0f); // discard outside radius 1
```

Create some data. The order of the data is defined with the hexgrid. Each hexgrid element has a 'vector iterator', `vi` and provides access to the location of the hex.
```c++
import sm.vvec;

sm::vvec<float> data (hg.num(), 0.0f);
for (auto h : hg.hexen) {
    data[h.vi] = some_function_of (h.x, h.y);
}
```

Create an sm::hexfft::fft object and perform a forward transform. The result is stored in `hfft.X_hexgrid`, which is a `sm::vvec` of `std::complex<>` values.

```c++
import sm.hexfft;

sm::hexfft::fft<float> hfft (&hg); // construct and initialize
hfft.forward (data); // Perform forward FFT transform
```
You can modify the values in `X_hexgrid` to make filters. The values in `X_hexgrid` are associated with a frequency hexgrid, `hexfft::fft::hgf`, which is created when hfft is initialized.
```c++
for (auto h : hfft.hgf->hexen) { // hgf is a unique_ptr to a hexgrid
    std::cout << "FFT Frequency " << h.x << ", " << h.y
              << " has magnitude " << std::real(hfft.X_hexgrid[h.vi]) << std::endl;
}

```
After changing values in `hfft.X_hexgrid` (perhaps by masking) you can then inverse transform from frequency space to image space

```c++
sm::vvec<std::complex<float>> invimg = hfft.inverse();
```
The returned data is defined over your original hexgrid, `hg`.

## Dependencies

sebsjames/mathplot and sebsjames/maths are implemented as C++ modules. This requires that you use clang-20 or higher and an up-to-date cmake in your toolchain.

If you are using Debian or Ubuntu, the following `apt` command should
install the mathplot dependencies.

```bash
sudo apt install build-essential cmake git ninja-build  \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev clang-20 clang-tools-20
```

If the cmake that you get from this apt command is too old, then it is easy (and fairly non-invasive) to compile and install it from source.

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
./fft_example # This loads ../bike256.png, so must be run from inside build/
```
