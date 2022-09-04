# Configuration Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# Set platform and debug mode if not already set.
platform ?= Ubuntu
debug    ?= 0
asan     ?= 0

# Define robot type if it isn't already set.
ROBOT_DEF ?= PMPCfg

# Set PHANToM and spacemouse compile.
phantom ?= 0
spacemouse ?= 0


# Directory Layout ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

VIZMO_DIR := .
PMPL_DIR  := $(VIZMO_DIR)/../pmpl/src
UTILS_DIR := $(VIZMO_DIR)/../pmpl/pmpl_utils
STAPL_DIR := $(UTILS_DIR)/stapl_release

# The OBJ_DIR is the landing zone for our object files. Create it with a
# directory structure to match our source tree.
OBJ_DIR   := build
MOC_DIR   := $(OBJ_DIR)/moc

# Treat all sub-directories except the build tree as possible source locations.
SRC_DIRS  := $(patsubst ./%,%, $(shell find . -type d \
               ! \( -path "./$(OBJ_DIR)*" -o -name '.' \) ))

# You might need to rebuild the build directory to catch newly added dirs.
$(OBJ_DIR):
	@mkdir -p $@ $(patsubst %,$(OBJ_DIR)/%,$(SRC_DIRS))
$(MOC_DIR): | $(OBJ_DIR)
	@mkdir $@

# Look for files in the source directories.
vpath %.h       $(SRC_DIRS)
vpath %.cpp     $(SRC_DIRS)
vpath moc_%.cpp $(MOC_DIR)


# External Makefiles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# include makefile defaults
include $(UTILS_DIR)/makefile_includes/Makefile.defaults
include Makefile.VizmoDefaults


# Object File Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# Pre-pend the build directory to the object file names.
VIZMO_OBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o, $(VIZMO_SRCS))

# Make the object files depend on the PMPL library so the latter is built first.
$(VIZMO_OBJS): $(MP_LIBFILE)

# Set which object defines the main function.
MAIN := $(OBJ_DIR)/main.o


# Dependency Tracking ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

DEPS_OUT = $(OBJ_DIR)/$*.d
DEPS     = -MMD -MF $(DEPS_OUT)

DEPENDENCY_FILE := $(OBJ_DIR)/Dependencies
-include $(DEPENDENCY_FILE)


# Library Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

VIZMO_LIBFILE = $(VIZMO_DIR)/libvizmo.a
$(VIZMO_LIBFILE): $(VIZMO_OBJS) $(MOC_OBJS) \
                | $(patsubst %,$(MOC_DIR)/%,$(MOC_SRCS))
	@echo Linking vizmo library...
	@$(AR) $@ $^


# Executable Recipes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# Default is to build the vizmo executable.
VIZMO_EXEC    := vizmo++
.DEFAULT_GOAL := $(VIZMO_EXEC)
$(VIZMO_EXEC): $(MAIN) $(VIZMO_LIBFILE)
	@echo Linking $@...
	@echo $(CXX) $(CXXFLAGS) $(OPTS) $^ $(LIBS) -o $@
	@$(CXX) $(CXXFLAGS) $(OPTS) $^ $(LIBS) -o $@


# Object File Recipes ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# This recipe runs headers with Q_OBJECT through the qt MOC.
$(MOC_DIR)/moc_%.cpp: %.h | $(MOC_DIR)
	@echo Moc\'ing $(shell echo $< | sed 's|.*/||' )...
	@mkdir -p $(MOC_DIR)/moc_$(shell dirname $<) #needed for MACOS support
	@echo $(QT_MOC) $< -o $@
	@$(QT_MOC) $< -o $@

# This recipe is for MOC sources.
$(MOC_DIR)/moc_%.o: $(MOC_DIR)/moc_%.cpp %.h | $(MOC_DIR)
	@echo Compiling Moc\'d source $(shell echo $< | sed 's|.*/||' )...
	@echo $(CXX) -c $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $(DEPS) $< -o $@
	@$(CXX) -c $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $(DEPS) $< -o $@
	@cat $(DEPS_OUT) >> $(DEPENDENCY_FILE)
	@rm -f $(DEPS_OUT)

# This recipe is for regular objects.
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	@$(MAKE) -s check_platform
	@echo Compiling $<...
	@echo $(CXX) -c $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $(DEPS) $< -o $@
	@$(CXX) -c $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $(DEPS) $< -o $@
	@cat $(DEPS_OUT) >> $(DEPENDENCY_FILE)
	@rm -f $(DEPS_OUT)


# Cleanup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.PHONY: clean
clean:
	@echo Cleaning vizmo library and executables...
	@rm -f $(VIZMO_LIBFILE) vizmo++

.PHONY: reallyclean
reallyclean: clean
	@echo Cleaning all vizmo objects...
	@rm -rf $(OBJ_DIR)

.PHONY: reallyreallyclean
reallyreallyclean: reallyclean
	@echo Cleaning all utilities...
	@cd $(PMPL_DIR) && $(MAKE) reallyreallyclean


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
