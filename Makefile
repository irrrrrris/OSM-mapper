# Copyright 2023 University of Toronto
# 
# Permission is hereby granted, to use this software and associated 
# documentation files (the "Software") in course work at the University 
# of Toronto, or for personal use. Other uses are prohibited, in 
# particular the distribution of the Software either publicly or to third 
# parties.
# 
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


################################################################################
#
# NOTE: It is always *your* responsibility to ensure your code works
#       with the autotester and can be submitted successfully.
#
#       We can not grade code that fails to compile/link.
#
#       After any changes verify that your code builds and submits successfully!
################################################################################


#If you wish to add custom linking/compile flags add them to the
# CUSTOM_LINK_FLAGS and CUSTOM_COMIPLE_FLAGS variables below
#
# By default the following custom flags are enabled:
#    * pkg-config is used to set all compile and linking flags required by EZGL and libcurl
#    * -fopenmp (OpenMP for parallel programming)
#
CUSTOM_COMPILE_FLAGS = $(shell pkg-config --cflags x11 gtk+-3.0 libcurl) -fopenmp 
CUSTOM_LINK_FLAGS    = $(shell pkg-config --libs x11 gtk+-3.0 libcurl) -fopenmp 

################################################################################
#	                ! WARNING - Here Be Dragons - WARNING !
#
#               Editting below this point should not be neccessary,
#                         and may break the autotester.
#
#                    Make sure you know what you are doing!
################################################################################

#Disable all predefined rules
.SUFFIXES:
MAKEFLAGS += r

#Allow automatic variables in dependencies
.SECONDEXPANSION:

################################################################################
# Utility Functions
################################################################################

#Recursive version of wildcard (i.e. it checks all subdirectories)
# For example:
# 	$(call rwildcard, /tmp/, *.c *.h)
# will return all files ending in .c or .h in /tmp and any of tmp's sub-directories
#NOTE: the directory (e.g. /tmp/) should end with a slash (i.e. /)
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

#Recursively find directories containing specifc files (useful for finding header include directories)
# For example:
# 	$(call rfiledirs, /tmp/, *.h *.hpp)
# will return all directories in /tmp which contain .h or .hpp files
#NOTE: the directory (e.g. /tmp/) should end with a slash (i.e. /)
rfiledirs=$(sort $(dir $(call rwildcard, $1, $2)))

################################################################################
# Configuration
################################################################################
#Default build type
CONF ?= release

$(info Configuration is '$(CONF)')

#
# Directories
#

#Directory for build temporaries
BUILDS_DIR = build
BUILD = $(BUILDS_DIR)/$(CONF)

#What directory contains the source files for the main executable?
EXE_SRC_DIR = main/src
#What directory contains the source files for the street map static library?
LIB_STREETMAP_SRC_DIR = libstreetmap/src/
#What directory contains the source files for the street map library tests?
LIB_STREETMAP_TEST_DIR = libstreetmap/tests/

#Global directory to look for custom library builds
ECE297_ROOT ?= /cad2/ece297s/public
ECE297_LIB_DIR ?= $(ECE297_ROOT)/lib/$(CONF)
ECE297_LIB_DIR_RELEASE ?= $(ECE297_ROOT)/lib/release
ECE297_INCLUDE_DIR ?= $(ECE297_ROOT)/include

#
#Output files
#

#Name of the primary executable
EXE=mapper
#Name of the test executable
LIB_STREETMAP_TEST=test_libstreetmap
#Name of the street map static library
LIB_STREETMAP=$(BUILD)/libstreetmap.a

#Things to build and copy to the project root
PRODUCTS=$(EXE) $(notdir $(LIB_STREETMAP))

################################################################################
# Tool flags
################################################################################

#What warning flags should be passed to the compiler?
#Turn on lots of warnings, but not signed/unsigned comparison as that
#produces nuisance warnings if you use int to loop over STL vectors etc.
WARN_CXXFLAGS = -Wall \
			 -Wextra \
			 -Wpedantic \
			 -Wcast-qual \
			 -Wcast-align \
			 -Wformat=2 \
			 -Wlogical-op \
			 -Wmissing-declarations \
			 -Wmissing-include-dirs \
			 -Wswitch-default \
			 -Wundef \
			 -Wunused-variable \
			 -Woverloaded-virtual \
			 -Wctor-dtor-privacy \
			 -Wnon-virtual-dtor \
			 -Wredundant-decls \
			 -Wshadow \
			 -Wno-sign-compare \
			 #-Wold-style-cast \
			 #-Wconversion \
			 #-Wno-sign-conversion

# Find all the include directoires for each component
ECE297_MILESTONE_INCLUDE_DIRS ?= $(ECE297_INCLUDE_DIR)/milestones
STREETS_DATABASE_INCLUDE_DIRS ?= $(ECE297_INCLUDE_DIR)/streetsdatabase
LIB_STREETMAP_INCLUDE_FLAGS = $(foreach dir, $(call rfiledirs, $(LIB_STREETMAP_SRC_DIR), *.h *.hpp), -I$(dir) -I$(dir)..)
EXE_INCLUDE_FLAGS = $(foreach dir, $(call rfiledirs, $(EXE_SRC_DIR), *.h *.hpp), -I$(dir))
STREETS_DATABASE_INCLUDE_FLAGS = $(foreach dir, $(STREETS_DATABASE_INCLUDE_DIRS), -I$(dir))
ECE297_MILESTONE_INCLUDE_FLAGS = $(foreach dir, $(ECE297_MILESTONE_INCLUDE_DIRS), -I$(dir))

#What flags should be passed to the compiler?
# Note: ECE297_LIB_DIR_RELEASE is a fallback
ECE297_CXXFLAGS = $(LIB_STREETMAP_INCLUDE_FLAGS) $(ECE297_MILESTONE_INCLUDE_FLAGS) $(STREETS_DATABASE_INCLUDE_FLAGS) $(EXE_INCLUDE_FLAGS)
ECE297_LDFLAGS = -L$(BUILD) -L$(ECE297_LIB_DIR) -L$(ECE297_LIB_DIR_RELEASE)

RELEASE_CXXFLAGS = -ggdb3 -O3
DEBUG_CXXFLAGS = -ggdb3 -O0
# Defining _GLIBCXX_DEBUG enables extra STL checking (e.g. vector [] bounds checking, iterator validity)
# Specifying -fsanitize enabled run-time sanitizer checking (e.g. bad memory access, leaks, undefined behaviour)
DEBUG_CHECK_CXXFLAGS = -ggdb3 -O0 -D_GLIBCXX_DEBUG -fsanitize=address -fsanitize=leak -fsanitize=undefined
DEBUG_CHECK_LDFLAGS = -fsanitize=address -fsanitize=leak -fsanitize=undefined
# It is helpful to disable function inlining when profiling so
# that the call-graph generated by profiling actually matches the
# source code. It also means that implicit actions like copy-constructors
# show up (since they are usually inlined). The trade-off is that the
# performance difference of such functions is likely over-emphasized, since
# inlining helps reduce thier overhead (to avoid this you can profile
# with the RELEASE build instead).
PROFILE_CXXFLAGS = -ggdb3 -O3 -fno-inline

ifeq (release, $(CONF))
	CONF_CXXFLAGS = $(RELEASE_CXXFLAGS)
else ifeq (debug, $(CONF))
	CONF_CXXFLAGS = $(DEBUG_CXXFLAGS)
else ifeq (debug_check, $(CONF))
	CONF_CXXFLAGS = $(DEBUG_CHECK_CXXFLAGS)
	CONF_LDFLAGS = $(DEBUG_CHECK_LDFLAGS)
else ifeq (profile, $(CONF))
	CONF_CXXFLAGS = $(PROFILE_CXXFLAGS)
else
$(error Invalid value for CONF: '$(CONF)', must be 'release', 'debug', 'debug_check', 'profile'. Try 'make help' for usage)
endif

#Collect all the options to give to the compiler
COMMON_CXXFLAGS = --std=c++17 $(CONF_CXXFLAGS) $(WARN_CXXFLAGS) $(ECE297_CXXFLAGS) $(CUSTOM_COMPILE_FLAGS) $(CXXFLAGS)
COMMON_LDFLAGS = $(CONF_LDFLAGS) $(ECE297_LDFLAGS) $(CUSTOM_LINK_FLAGS) $(LDFLAGS)
COMMON_LDLIBS = -lstreetsdatabase -lboost_serialization $(LDLIBS)
TEST_LDLIBS = -lUnitTest++ $(COMMON_LDLIBS)

################################################################################
# Generate object file names from source file names
################################################################################

#Objects associated with the main executable
EXE_OBJ=$(patsubst %.cpp, $(BUILD)/%.o, $(call rwildcard, $(EXE_SRC_DIR), *.cpp))

#Objects associated with the street map library
LIB_STREETMAP_OBJ=$(patsubst %.cpp, $(BUILD)/%.o, $(call rwildcard, $(LIB_STREETMAP_SRC_DIR), *.cpp))

#Objects associated with tests for the street map library
# We collect tests both from the local project (under LIB_STREETMAP_TEST_DIR)
LIB_STREETMAP_TEST_OBJ=$(patsubst %.cpp, $(BUILD)/%.o, \
					   	$(call rwildcard, $(LIB_STREETMAP_TEST_DIR), *.cpp) \
					   )

################################################################################
# Dependency files
################################################################################

#To capture dependencies on header files,
# we told the compiler to generate dependency 
# files associated with each object file
#
#The ':.o=.d' syntax means replace each filename ending in .o with .d
# For example:
#   build/main/main.o would become build/main/main.d
DEP = $(EXE_OBJ:.o=.d) $(LIB_STREETMAP_OBJ:.o=.d) $(LIB_STREETMAP_TEST_OBJ:.o=.d)

################################################################################
# Make targets
################################################################################

#Phony targets are always run
.PHONY: \
	clean all test \
	echo_flags help \
	$(PRODUCTS) \

#The default target
# This is called when you type 'make' on the command line
all: $(PRODUCTS)

#This builds and runs the unit test executable
test: $(LIB_STREETMAP_TEST)
	@echo ""
	@echo "Running Unit Tests..."
	$(LIB_STREETMAP_TEST)

#Symlink the test exec to the project root
$(LIB_STREETMAP_TEST): $$(BUILD)/$$@
	@rm -f $@
	ln -s $< $@

#Symlink the products to the project root
$(PRODUCTS): $$(BUILD)/$$@
	@rm -f $@
	ln -s $< $@

#Include header file dependencies generated by a
# previous compile
-include $(DEP)

#Link main executable
$(BUILD)/$(EXE): $(EXE_OBJ) $(LIB_STREETMAP)
	$(CXX) -o $@ $^ $(COMMON_LDFLAGS) $(COMMON_LDLIBS)

#Link test executable
$(BUILD)/$(LIB_STREETMAP_TEST): $(LIB_STREETMAP_TEST_OBJ) $(LIB_STREETMAP)
	$(CXX) -o $@ $^ $(COMMON_LDFLAGS) $(TEST_LDLIBS)

#Street Map static library
$(LIB_STREETMAP): $(LIB_STREETMAP_OBJ)
	@mkdir -p $(@D)
	$(AR) crs $@ $^

#Build a C++ file
$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< -MMD -MP $(COMMON_CXXFLAGS)

clean:
	rm -rf $(BUILDS_DIR)
	rm -f $(EXE) $(LIB_STREETMAP_TEST)

echo_flags:
	@echo "CUSTOM_COMPILE_FLAGS: $(CUSTOM_COMPILE_FLAGS)"
	@echo "CUSTOM_LINK_FLAGS: $(CUSTOM_LINK_FLAGS)"
	@echo "COMPILE_FLAGS: $(COMMON_CXXFLAGS)"
	@echo "LINK_FLAGS: $(COMMON_LDFLAGS) $(COMMON_LDLIBS)"

help:
	@echo "Makefile for ECE297"
	@echo ""
	@echo "Usage: "
	@echo '    > make -j4'
	@echo "        Call the default make target (all)."
	@echo "        This builds the project executable: '$(EXE)'."
	@echo "        Use -j4 option to do parallel builds."
	@echo "    > make clean"
	@echo "        Removes any generated files including exectuables,"
	@echo "        static libraries, and object files."
	@echo "    > make test"
	@echo "        Builds and runs unit tests."
	@echo "        Builds and runs any tests found in $(LIB_STREETMAP_TEST_DIR),"
	@echo "        generating the test executable '$(LIB_STREETMAP_TEST)'."
	@echo "    > make echo_flags"
	@echo "        Echos the compile and link flags used by the Makefile."
	@echo "    > make help"
	@echo "        Prints this help message."
	@echo ""
	@echo ""
	@echo "Configuration Variables: "
	@echo "    CONF={release | debug | debug_check | profile}"
	@echo "        Controls whether the build performs compiler optimizations"
	@echo "        to improve performance. Currently set to '$(CONF)'."
	@echo ""
	@echo "        With CONF=release compiler optimization is enabled ($(RELEASE_CXXFLAGS))."
	@echo ""
	@echo "        With CONF=debug compiler optimization is disabled to improve"
	@echo "        interactive debugging ($(DEBUG_CXXFLAGS))."
	@echo ""
	@echo "        With CONF=debug_check compiler optimization is disabled, and extra"
	@echo "        error checking enabled for the standard library, invalid memory access"
	@echo "        and undefined behaviour ($(DEBUG_CHECK_CXXFLAGS))."
	@echo ""
	@echo "        With CONF=profile a release-like build with gprof profiling "
	@echo "        enabled ($(PROFILE_CXXFLAGS))."
	@echo ""
	@echo "        You can configure specify this option on the command line."
	@echo "        To perform a release build you can use: "
	@echo "            > make CONF=release"
	@echo "        To perform a regular debug build you can use: "
	@echo "            > make CONF=debug"
	@echo "        To perform a debug build with extra checking (runs slower) you can use: "
	@echo "            > make CONF=debug_check"
	@echo "        To perform a profile build you can use: "
	@echo "            > make CONF=profile"
