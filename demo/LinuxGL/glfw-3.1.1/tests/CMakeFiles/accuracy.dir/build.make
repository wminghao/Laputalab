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
include tests/CMakeFiles/accuracy.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/accuracy.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/accuracy.dir/flags.make

tests/CMakeFiles/accuracy.dir/accuracy.c.o: tests/CMakeFiles/accuracy.dir/flags.make
tests/CMakeFiles/accuracy.dir/accuracy.c.o: tests/accuracy.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Laputalab/demo/LinuxGL/glfw-3.1.1/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/accuracy.dir/accuracy.c.o"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/accuracy.dir/accuracy.c.o   -c /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/accuracy.c

tests/CMakeFiles/accuracy.dir/accuracy.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/accuracy.dir/accuracy.c.i"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/accuracy.c > CMakeFiles/accuracy.dir/accuracy.c.i

tests/CMakeFiles/accuracy.dir/accuracy.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/accuracy.dir/accuracy.c.s"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/accuracy.c -o CMakeFiles/accuracy.dir/accuracy.c.s

tests/CMakeFiles/accuracy.dir/accuracy.c.o.requires:
.PHONY : tests/CMakeFiles/accuracy.dir/accuracy.c.o.requires

tests/CMakeFiles/accuracy.dir/accuracy.c.o.provides: tests/CMakeFiles/accuracy.dir/accuracy.c.o.requires
	$(MAKE) -f tests/CMakeFiles/accuracy.dir/build.make tests/CMakeFiles/accuracy.dir/accuracy.c.o.provides.build
.PHONY : tests/CMakeFiles/accuracy.dir/accuracy.c.o.provides

tests/CMakeFiles/accuracy.dir/accuracy.c.o.provides.build: tests/CMakeFiles/accuracy.dir/accuracy.c.o

# Object files for target accuracy
accuracy_OBJECTS = \
"CMakeFiles/accuracy.dir/accuracy.c.o"

# External object files for target accuracy
accuracy_EXTERNAL_OBJECTS =

tests/accuracy: tests/CMakeFiles/accuracy.dir/accuracy.c.o
tests/accuracy: tests/CMakeFiles/accuracy.dir/build.make
tests/accuracy: src/libglfw3.a
tests/accuracy: /usr/lib/x86_64-linux-gnu/libX11.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/libXrandr.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/libXinerama.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/libXi.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/libXxf86vm.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/librt.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/libm.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/libXcursor.so
tests/accuracy: /usr/lib/x86_64-linux-gnu/libGL.so
tests/accuracy: tests/CMakeFiles/accuracy.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable accuracy"
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/accuracy.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/accuracy.dir/build: tests/accuracy
.PHONY : tests/CMakeFiles/accuracy.dir/build

tests/CMakeFiles/accuracy.dir/requires: tests/CMakeFiles/accuracy.dir/accuracy.c.o.requires
.PHONY : tests/CMakeFiles/accuracy.dir/requires

tests/CMakeFiles/accuracy.dir/clean:
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1/tests && $(CMAKE_COMMAND) -P CMakeFiles/accuracy.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/accuracy.dir/clean

tests/CMakeFiles/accuracy.dir/depend:
	cd /Laputalab/demo/LinuxGL/glfw-3.1.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Laputalab/demo/LinuxGL/glfw-3.1.1 /Laputalab/demo/LinuxGL/glfw-3.1.1/tests /Laputalab/demo/LinuxGL/glfw-3.1.1 /Laputalab/demo/LinuxGL/glfw-3.1.1/tests /Laputalab/demo/LinuxGL/glfw-3.1.1/tests/CMakeFiles/accuracy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/accuracy.dir/depend

