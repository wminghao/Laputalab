# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

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
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1

# Include any dependencies generated for this target.
include tests/CMakeFiles/accuracy.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/accuracy.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/accuracy.dir/flags.make

tests/CMakeFiles/accuracy.dir/accuracy.c.o: tests/CMakeFiles/accuracy.dir/flags.make
tests/CMakeFiles/accuracy.dir/accuracy.c.o: tests/accuracy.c
	$(CMAKE_COMMAND) -E cmake_progress_report /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object tests/CMakeFiles/accuracy.dir/accuracy.c.o"
	cd /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/accuracy.dir/accuracy.c.o   -c /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/accuracy.c

tests/CMakeFiles/accuracy.dir/accuracy.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/accuracy.dir/accuracy.c.i"
	cd /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/accuracy.c > CMakeFiles/accuracy.dir/accuracy.c.i

tests/CMakeFiles/accuracy.dir/accuracy.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/accuracy.dir/accuracy.c.s"
	cd /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/accuracy.c -o CMakeFiles/accuracy.dir/accuracy.c.s

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

tests/accuracy.app/Contents/MacOS/accuracy: tests/CMakeFiles/accuracy.dir/accuracy.c.o
tests/accuracy.app/Contents/MacOS/accuracy: tests/CMakeFiles/accuracy.dir/build.make
tests/accuracy.app/Contents/MacOS/accuracy: src/libglfw3.a
tests/accuracy.app/Contents/MacOS/accuracy: tests/CMakeFiles/accuracy.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable accuracy.app/Contents/MacOS/accuracy"
	cd /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/accuracy.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/accuracy.dir/build: tests/accuracy.app/Contents/MacOS/accuracy
.PHONY : tests/CMakeFiles/accuracy.dir/build

tests/CMakeFiles/accuracy.dir/requires: tests/CMakeFiles/accuracy.dir/accuracy.c.o.requires
.PHONY : tests/CMakeFiles/accuracy.dir/requires

tests/CMakeFiles/accuracy.dir/clean:
	cd /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests && $(CMAKE_COMMAND) -P CMakeFiles/accuracy.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/accuracy.dir/clean

tests/CMakeFiles/accuracy.dir/depend:
	cd /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1 /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests /Users/howard/AR/demo/LaputaDesktop3/thirdparty/glfw-3.1-2.1/tests/CMakeFiles/accuracy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/accuracy.dir/depend

