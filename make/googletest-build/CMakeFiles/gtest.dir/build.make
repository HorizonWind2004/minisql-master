# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.29.3/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.29.3/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/xieji/Code/minisql-master

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/xieji/Code/minisql-master/make

# Include any dependencies generated for this target.
include googletest-build/CMakeFiles/gtest.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include googletest-build/CMakeFiles/gtest.dir/compiler_depend.make

# Include the progress variables for this target.
include googletest-build/CMakeFiles/gtest.dir/progress.make

# Include the compile flags for this target's objects.
include googletest-build/CMakeFiles/gtest.dir/flags.make

googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.o: googletest-build/CMakeFiles/gtest.dir/flags.make
googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.o: /Users/xieji/Code/minisql-master/thirdparty/googletest/src/gtest-all.cc
googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.o: googletest-build/CMakeFiles/gtest.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/xieji/Code/minisql-master/make/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.o"
	cd /Users/xieji/Code/minisql-master/make/googletest-build && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.o -MF CMakeFiles/gtest.dir/src/gtest-all.cc.o.d -o CMakeFiles/gtest.dir/src/gtest-all.cc.o -c /Users/xieji/Code/minisql-master/thirdparty/googletest/src/gtest-all.cc

googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/gtest.dir/src/gtest-all.cc.i"
	cd /Users/xieji/Code/minisql-master/make/googletest-build && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/xieji/Code/minisql-master/thirdparty/googletest/src/gtest-all.cc > CMakeFiles/gtest.dir/src/gtest-all.cc.i

googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/gtest.dir/src/gtest-all.cc.s"
	cd /Users/xieji/Code/minisql-master/make/googletest-build && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/xieji/Code/minisql-master/thirdparty/googletest/src/gtest-all.cc -o CMakeFiles/gtest.dir/src/gtest-all.cc.s

# Object files for target gtest
gtest_OBJECTS = \
"CMakeFiles/gtest.dir/src/gtest-all.cc.o"

# External object files for target gtest
gtest_EXTERNAL_OBJECTS =

lib/libgtest.1.11.0.dylib: googletest-build/CMakeFiles/gtest.dir/src/gtest-all.cc.o
lib/libgtest.1.11.0.dylib: googletest-build/CMakeFiles/gtest.dir/build.make
lib/libgtest.1.11.0.dylib: googletest-build/CMakeFiles/gtest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/xieji/Code/minisql-master/make/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library ../lib/libgtest.dylib"
	cd /Users/xieji/Code/minisql-master/make/googletest-build && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gtest.dir/link.txt --verbose=$(VERBOSE)
	cd /Users/xieji/Code/minisql-master/make/googletest-build && $(CMAKE_COMMAND) -E cmake_symlink_library ../lib/libgtest.1.11.0.dylib ../lib/libgtest.1.11.0.dylib ../lib/libgtest.dylib

lib/libgtest.dylib: lib/libgtest.1.11.0.dylib
	@$(CMAKE_COMMAND) -E touch_nocreate lib/libgtest.dylib

# Rule to build all files generated by this target.
googletest-build/CMakeFiles/gtest.dir/build: lib/libgtest.dylib
.PHONY : googletest-build/CMakeFiles/gtest.dir/build

googletest-build/CMakeFiles/gtest.dir/clean:
	cd /Users/xieji/Code/minisql-master/make/googletest-build && $(CMAKE_COMMAND) -P CMakeFiles/gtest.dir/cmake_clean.cmake
.PHONY : googletest-build/CMakeFiles/gtest.dir/clean

googletest-build/CMakeFiles/gtest.dir/depend:
	cd /Users/xieji/Code/minisql-master/make && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/xieji/Code/minisql-master /Users/xieji/Code/minisql-master/thirdparty/googletest /Users/xieji/Code/minisql-master/make /Users/xieji/Code/minisql-master/make/googletest-build /Users/xieji/Code/minisql-master/make/googletest-build/CMakeFiles/gtest.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : googletest-build/CMakeFiles/gtest.dir/depend
