#
# This is an example CMakeLists.txt file to compile a program against mathplot
#
# Mathplot provides all its functionality as modules. What you do is
# compile these modules - the mathplot core - into a library. You then
# compile your own program, importing the mathplot features you need
# and linking to this library.
#
cmake_minimum_required(VERSION 3.28.5 FATAL_ERROR)

# Give your project a name. Specify language CXX (for maths, mathplot and your code) and also C (for glad)
project(mathplot_template LANGUAGES CXX C)

# mathplot uses c++-20 language features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Find the libraries which will be needed by mathplot
#

set(OpenGL_GL_PREFERENCE "GLVND") # Following `cmake --help-policy CMP0072`
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

#
# Define locations of our three submodules, located in extern/
#

set(MPLOT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/extern/mathplot" CACHE PATH "The path to sebsjames/mathplot")
set(MATHS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/extern/maths" CACHE PATH "The path to sebsjames/maths")
set(JSON_SOURCE_DIR "${PROJECT_SOURCE_DIR}/extern/json" CACHE PATH "The path to nlohmann/json")

#
# Set compiler flags
#

# Assume a gcc compiler (or a gcc mimic like Clang).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3 -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors")

# Tell the program where the mathplot fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPLOT_FONTS_DIR=\"\\\"${MPLOT_SOURCE_DIR}/fonts\\\"\"")

# Additional GL compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")

#
# Include paths for module compilation
#

# mathplot needs these includes
include_directories(${OPENGL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${MPLOT_SOURCE_DIR}/mplot/glad)

# This is required for compilation of the nlohmann/json module
include_directories ("${JSON_SOURCE_DIR}/include")

#
# Define lists of module files to build. We pull in some definitions
# of variables that list the modules in maths and mathplot
#

# Include maths module defs by running the sebsjames/maths macro 'setup_module_variables_for_maths'
include(${MATHS_SOURCE_DIR}/cmake/module_definitions.cmake)
setup_module_variables_for_maths (${MATHS_SOURCE_DIR} ${JSON_SOURCE_DIR})
# Include mathplot module defs by running two macros from sebsjames/mathplot
include(${MPLOT_SOURCE_DIR}/cmake/module_definitions.cmake)
setup_module_variables_for_mathplot (${MPLOT_SOURCE_DIR} ${MATHS_SOURCE_DIR} ${JSON_SOURCE_DIR})


#
# Compile two 'boilerplate' libraries - glad and the mathplot fontdata
#

# For mathplot, we must build the glad library here
add_library(glad STATIC ${MPLOT_SOURCE_DIR}/mplot/glad/gl.c)

# We have to build the mathplot fonts data library - this packs up
# some TTF fonts into an object file; they're unpacked at runtime.
add_library(mplot_fontdata STATIC ${MPLOT_SOURCE_DIR}/mplot/VisualFaceAsm.cpp)

#
# Compile a library with the mathplot features we want
#

# Define a list of the modules we wish to compile into our library
set(MY_MATHPLOT_MODULES
  ${MPLOT_CORE_MODULES}
  ${MPLOT_SOURCE_DIR}/mplot/GraphVisual.cppm  # The example uses a GraphVisual
)
# De-duplicate the list
list(REMOVE_DUPLICATES MATHPLOT_MODULES)
# Add a library called mathplot_modules
add_library(mathplot_modules SHARED)
# Attach the list of modules to the library
target_sources(mathplot_modules
  PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${MY_MATHPLOT_MODULES}
)
# Link the OpenGL, Freetype and GLFW libraries to this internal library
target_link_libraries(mathplot_modules OpenGL::GL Freetype::Freetype glfw glad mplot_fontdata)

#
# Now it's time to define how we will compile the program prog1
#

# The sample is compiled from a single c++ file
add_executable(prog1 prog1.cpp)

# Link our internal library mathplot_modules
target_link_libraries(prog1 mathplot_modules)

#
# All done!
#
