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
CMAKE_SOURCE_DIR = /home/chris/validation/code

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/chris/validation/code/build

# Utility rule file for NightlySubmit.

# Include the progress variables for this target.
include stxxl/CMakeFiles/NightlySubmit.dir/progress.make

stxxl/CMakeFiles/NightlySubmit:
	cd /home/chris/validation/code/build/stxxl && /usr/bin/ctest -D NightlySubmit

NightlySubmit: stxxl/CMakeFiles/NightlySubmit
NightlySubmit: stxxl/CMakeFiles/NightlySubmit.dir/build.make
.PHONY : NightlySubmit

# Rule to build all files generated by this target.
stxxl/CMakeFiles/NightlySubmit.dir/build: NightlySubmit
.PHONY : stxxl/CMakeFiles/NightlySubmit.dir/build

stxxl/CMakeFiles/NightlySubmit.dir/clean:
	cd /home/chris/validation/code/build/stxxl && $(CMAKE_COMMAND) -P CMakeFiles/NightlySubmit.dir/cmake_clean.cmake
.PHONY : stxxl/CMakeFiles/NightlySubmit.dir/clean

stxxl/CMakeFiles/NightlySubmit.dir/depend:
	cd /home/chris/validation/code/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/chris/validation/code /home/chris/validation/code/stxxl /home/chris/validation/code/build /home/chris/validation/code/build/stxxl /home/chris/validation/code/build/stxxl/CMakeFiles/NightlySubmit.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : stxxl/CMakeFiles/NightlySubmit.dir/depend

