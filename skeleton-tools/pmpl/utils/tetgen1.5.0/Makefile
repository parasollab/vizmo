# External Makefiles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

include ../makefile_includes/Makefile.defaults

# Compiler Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

FLAGS := -std=c++11 -O2
DEFS  := -DTETLIBRARY

# Object File Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

OBJECTS := predicates.o tetgen.o

%.o: %.cxx
	@$(MAKE) -s check_platform
	@echo Compiling $@...
	@$(CXX) -c $(FLAGS) $(DEFS) $< -o $@

# Library Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# Default target is the tetgen library.
.DEFAULT_GOAL = $(LIBRARY)

LIBRARY := libtet.a
$(LIBRARY): $(OBJECTS)
	@echo Linking tetgen library...
	@$(AR) $@ $^

# Cleanup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.PHONY: clean
clean:
	@echo Cleaning tetgen library...
	@rm -f $(OBJECTS) $(LIBRARY)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
