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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/baby/meshserver/minimal-ws-server-echo

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/baby/meshserver/minimal-ws-server-echo/build

# Include any dependencies generated for this target.
include CMakeFiles/lws-minimal-ws-server-echo.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/lws-minimal-ws-server-echo.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/lws-minimal-ws-server-echo.dir/flags.make

CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o: CMakeFiles/lws-minimal-ws-server-echo.dir/flags.make
CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o: ../minimal-ws-server-echo.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/baby/meshserver/minimal-ws-server-echo/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o   -c /home/baby/meshserver/minimal-ws-server-echo/minimal-ws-server-echo.c

CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/baby/meshserver/minimal-ws-server-echo/minimal-ws-server-echo.c > CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.i

CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/baby/meshserver/minimal-ws-server-echo/minimal-ws-server-echo.c -o CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.s

CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.requires:
.PHONY : CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.requires

CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.provides: CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.requires
	$(MAKE) -f CMakeFiles/lws-minimal-ws-server-echo.dir/build.make CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.provides.build
.PHONY : CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.provides

CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.provides.build: CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o

# Object files for target lws-minimal-ws-server-echo
lws__minimal__ws__server__echo_OBJECTS = \
"CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o"

# External object files for target lws-minimal-ws-server-echo
lws__minimal__ws__server__echo_EXTERNAL_OBJECTS =

lws-minimal-ws-server-echo: CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o
lws-minimal-ws-server-echo: CMakeFiles/lws-minimal-ws-server-echo.dir/build.make
lws-minimal-ws-server-echo: /usr/local/lib/libwebsockets.a
lws-minimal-ws-server-echo: /usr/lib64/libssl.so
lws-minimal-ws-server-echo: /usr/lib64/libcrypto.so
lws-minimal-ws-server-echo: CMakeFiles/lws-minimal-ws-server-echo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable lws-minimal-ws-server-echo"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lws-minimal-ws-server-echo.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/lws-minimal-ws-server-echo.dir/build: lws-minimal-ws-server-echo
.PHONY : CMakeFiles/lws-minimal-ws-server-echo.dir/build

CMakeFiles/lws-minimal-ws-server-echo.dir/requires: CMakeFiles/lws-minimal-ws-server-echo.dir/minimal-ws-server-echo.c.o.requires
.PHONY : CMakeFiles/lws-minimal-ws-server-echo.dir/requires

CMakeFiles/lws-minimal-ws-server-echo.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/lws-minimal-ws-server-echo.dir/cmake_clean.cmake
.PHONY : CMakeFiles/lws-minimal-ws-server-echo.dir/clean

CMakeFiles/lws-minimal-ws-server-echo.dir/depend:
	cd /home/baby/meshserver/minimal-ws-server-echo/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/baby/meshserver/minimal-ws-server-echo /home/baby/meshserver/minimal-ws-server-echo /home/baby/meshserver/minimal-ws-server-echo/build /home/baby/meshserver/minimal-ws-server-echo/build /home/baby/meshserver/minimal-ws-server-echo/build/CMakeFiles/lws-minimal-ws-server-echo.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/lws-minimal-ws-server-echo.dir/depend
