# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1

# Include any dependencies generated for this target.
include tests/CMakeFiles/defaults.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/defaults.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/defaults.dir/flags.make

tests/CMakeFiles/defaults.dir/defaults.c.o: tests/CMakeFiles/defaults.dir/flags.make
tests/CMakeFiles/defaults.dir/defaults.c.o: tests/defaults.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/defaults.dir/defaults.c.o"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/defaults.dir/defaults.c.o   -c /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/defaults.c

tests/CMakeFiles/defaults.dir/defaults.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/defaults.dir/defaults.c.i"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/defaults.c > CMakeFiles/defaults.dir/defaults.c.i

tests/CMakeFiles/defaults.dir/defaults.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/defaults.dir/defaults.c.s"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/defaults.c -o CMakeFiles/defaults.dir/defaults.c.s

tests/CMakeFiles/defaults.dir/defaults.c.o.requires:
.PHONY : tests/CMakeFiles/defaults.dir/defaults.c.o.requires

tests/CMakeFiles/defaults.dir/defaults.c.o.provides: tests/CMakeFiles/defaults.dir/defaults.c.o.requires
	$(MAKE) -f tests/CMakeFiles/defaults.dir/build.make tests/CMakeFiles/defaults.dir/defaults.c.o.provides.build
.PHONY : tests/CMakeFiles/defaults.dir/defaults.c.o.provides

tests/CMakeFiles/defaults.dir/defaults.c.o.provides.build: tests/CMakeFiles/defaults.dir/defaults.c.o

# Object files for target defaults
defaults_OBJECTS = \
"CMakeFiles/defaults.dir/defaults.c.o"

# External object files for target defaults
defaults_EXTERNAL_OBJECTS =

tests/defaults: tests/CMakeFiles/defaults.dir/defaults.c.o
tests/defaults: tests/CMakeFiles/defaults.dir/build.make
tests/defaults: src/libglfw3.a
tests/defaults: /usr/lib/x86_64-linux-gnu/libX11.so
tests/defaults: /usr/lib/x86_64-linux-gnu/libXrandr.so
tests/defaults: /usr/lib/x86_64-linux-gnu/libXinerama.so
tests/defaults: /usr/lib/x86_64-linux-gnu/libXi.so
tests/defaults: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
tests/defaults: /usr/lib/x86_64-linux-gnu/librt.so
tests/defaults: /usr/lib/x86_64-linux-gnu/libm.so
tests/defaults: /usr/lib/x86_64-linux-gnu/libXcursor.so
tests/defaults: /usr/lib/x86_64-linux-gnu/libGL.so
tests/defaults: tests/CMakeFiles/defaults.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable defaults"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/defaults.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/defaults.dir/build: tests/defaults
.PHONY : tests/CMakeFiles/defaults.dir/build

tests/CMakeFiles/defaults.dir/requires: tests/CMakeFiles/defaults.dir/defaults.c.o.requires
.PHONY : tests/CMakeFiles/defaults.dir/requires

tests/CMakeFiles/defaults.dir/clean:
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && $(CMAKE_COMMAND) -P CMakeFiles/defaults.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/defaults.dir/clean

tests/CMakeFiles/defaults.dir/depend:
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/CMakeFiles/defaults.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/defaults.dir/depend

