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
include tests/CMakeFiles/gamma.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/gamma.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/gamma.dir/flags.make

tests/CMakeFiles/gamma.dir/gamma.c.o: tests/CMakeFiles/gamma.dir/flags.make
tests/CMakeFiles/gamma.dir/gamma.c.o: tests/gamma.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/gamma.dir/gamma.c.o"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/gamma.dir/gamma.c.o   -c /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/gamma.c

tests/CMakeFiles/gamma.dir/gamma.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gamma.dir/gamma.c.i"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/gamma.c > CMakeFiles/gamma.dir/gamma.c.i

tests/CMakeFiles/gamma.dir/gamma.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gamma.dir/gamma.c.s"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/gamma.c -o CMakeFiles/gamma.dir/gamma.c.s

tests/CMakeFiles/gamma.dir/gamma.c.o.requires:
.PHONY : tests/CMakeFiles/gamma.dir/gamma.c.o.requires

tests/CMakeFiles/gamma.dir/gamma.c.o.provides: tests/CMakeFiles/gamma.dir/gamma.c.o.requires
	$(MAKE) -f tests/CMakeFiles/gamma.dir/build.make tests/CMakeFiles/gamma.dir/gamma.c.o.provides.build
.PHONY : tests/CMakeFiles/gamma.dir/gamma.c.o.provides

tests/CMakeFiles/gamma.dir/gamma.c.o.provides.build: tests/CMakeFiles/gamma.dir/gamma.c.o

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o: tests/CMakeFiles/gamma.dir/flags.make
tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o: deps/getopt.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/gamma.dir/__/deps/getopt.c.o   -c /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/deps/getopt.c

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/gamma.dir/__/deps/getopt.c.i"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/deps/getopt.c > CMakeFiles/gamma.dir/__/deps/getopt.c.i

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/gamma.dir/__/deps/getopt.c.s"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/deps/getopt.c -o CMakeFiles/gamma.dir/__/deps/getopt.c.s

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires:
.PHONY : tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires
	$(MAKE) -f tests/CMakeFiles/gamma.dir/build.make tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides.build
.PHONY : tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides

tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.provides.build: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o

# Object files for target gamma
gamma_OBJECTS = \
"CMakeFiles/gamma.dir/gamma.c.o" \
"CMakeFiles/gamma.dir/__/deps/getopt.c.o"

# External object files for target gamma
gamma_EXTERNAL_OBJECTS =

tests/gamma: tests/CMakeFiles/gamma.dir/gamma.c.o
tests/gamma: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o
tests/gamma: tests/CMakeFiles/gamma.dir/build.make
tests/gamma: src/libglfw3.a
tests/gamma: /usr/lib/x86_64-linux-gnu/libX11.so
tests/gamma: /usr/lib/x86_64-linux-gnu/libXrandr.so
tests/gamma: /usr/lib/x86_64-linux-gnu/libXinerama.so
tests/gamma: /usr/lib/x86_64-linux-gnu/libXi.so
tests/gamma: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
tests/gamma: /usr/lib/x86_64-linux-gnu/librt.so
tests/gamma: /usr/lib/x86_64-linux-gnu/libm.so
tests/gamma: /usr/lib/x86_64-linux-gnu/libXcursor.so
tests/gamma: /usr/lib/x86_64-linux-gnu/libGL.so
tests/gamma: tests/CMakeFiles/gamma.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable gamma"
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gamma.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/gamma.dir/build: tests/gamma
.PHONY : tests/CMakeFiles/gamma.dir/build

tests/CMakeFiles/gamma.dir/requires: tests/CMakeFiles/gamma.dir/gamma.c.o.requires
tests/CMakeFiles/gamma.dir/requires: tests/CMakeFiles/gamma.dir/__/deps/getopt.c.o.requires
.PHONY : tests/CMakeFiles/gamma.dir/requires

tests/CMakeFiles/gamma.dir/clean:
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && $(CMAKE_COMMAND) -P CMakeFiles/gamma.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/gamma.dir/clean

tests/CMakeFiles/gamma.dir/depend:
	cd /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests /Laputalab/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/CMakeFiles/gamma.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/gamma.dir/depend

