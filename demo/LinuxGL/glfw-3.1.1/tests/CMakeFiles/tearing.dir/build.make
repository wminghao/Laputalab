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
CMAKE_SOURCE_DIR = /Laputalab/demo/LinuxGL/glfw-3.1.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Laputalab/demo/LinuxGL/glfw-3.1.1

# Include any dependencies generated for this target.
include tests/CMakeFiles/tearing.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/tearing.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/tearing.dir/flags.make

tests/CMakeFiles/tearing.dir/tearing.c.o: tests/CMakeFiles/tearing.dir/flags.make
tests/CMakeFiles/tearing.dir/tearing.c.o: tests/tearing.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Laputalab/demo/LinuxGL/glfw-3.1.1/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/tearing.dir/tearing.c.o"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/tearing.dir/tearing.c.o   -c /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/tearing.c

tests/CMakeFiles/tearing.dir/tearing.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/tearing.dir/tearing.c.i"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/tearing.c > CMakeFiles/tearing.dir/tearing.c.i

tests/CMakeFiles/tearing.dir/tearing.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/tearing.dir/tearing.c.s"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/tearing.c -o CMakeFiles/tearing.dir/tearing.c.s

tests/CMakeFiles/tearing.dir/tearing.c.o.requires:
.PHONY : tests/CMakeFiles/tearing.dir/tearing.c.o.requires

tests/CMakeFiles/tearing.dir/tearing.c.o.provides: tests/CMakeFiles/tearing.dir/tearing.c.o.requires
	$(MAKE) -f tests/CMakeFiles/tearing.dir/build.make tests/CMakeFiles/tearing.dir/tearing.c.o.provides.build
.PHONY : tests/CMakeFiles/tearing.dir/tearing.c.o.provides

tests/CMakeFiles/tearing.dir/tearing.c.o.provides.build: tests/CMakeFiles/tearing.dir/tearing.c.o

# Object files for target tearing
tearing_OBJECTS = \
"CMakeFiles/tearing.dir/tearing.c.o"

# External object files for target tearing
tearing_EXTERNAL_OBJECTS =

tests/tearing: tests/CMakeFiles/tearing.dir/tearing.c.o
tests/tearing: tests/CMakeFiles/tearing.dir/build.make
tests/tearing: src/libglfw3.a
tests/tearing: /usr/lib/x86_64-linux-gnu/libX11.so
tests/tearing: /usr/lib/x86_64-linux-gnu/libXrandr.so
tests/tearing: /usr/lib/x86_64-linux-gnu/libXinerama.so
tests/tearing: /usr/lib/x86_64-linux-gnu/libXi.so
tests/tearing: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
tests/tearing: /usr/lib/x86_64-linux-gnu/librt.so
tests/tearing: /usr/lib/x86_64-linux-gnu/libm.so
tests/tearing: /usr/lib/x86_64-linux-gnu/libXcursor.so
tests/tearing: /usr/lib/x86_64-linux-gnu/libGL.so
tests/tearing: tests/CMakeFiles/tearing.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable tearing"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tearing.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/tearing.dir/build: tests/tearing
.PHONY : tests/CMakeFiles/tearing.dir/build

tests/CMakeFiles/tearing.dir/requires: tests/CMakeFiles/tearing.dir/tearing.c.o.requires
.PHONY : tests/CMakeFiles/tearing.dir/requires

tests/CMakeFiles/tearing.dir/clean:
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && $(CMAKE_COMMAND) -P CMakeFiles/tearing.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/tearing.dir/clean

tests/CMakeFiles/tearing.dir/depend:
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Laputalab/demo/LinuxGL/glfw-3.1.1 /Laputalab/demo/LinuxGL/glfw-3.1.1/tests /Laputalab/demo/LinuxGL/glfw-3.1.1 /Laputalab/demo/LinuxGL/glfw-3.1.1/tests /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/CMakeFiles/tearing.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/tearing.dir/depend

