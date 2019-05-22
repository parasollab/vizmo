# External Makefiles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

include ../../../makefile_includes/Makefile.defaults

# Compiler Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

INCL := -I. -I../include

# Object File Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

OBJECTS := ANN.o kd_tree.o kd_util.o kd_split.o kd_search.o nn.o multiann.o

%.o: %.C
	@$(MAKE) -s check_platform
	@echo Compiling $@...
	@$(CXX) -c $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $< -o $@

# Library Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.DEFAULT_GOAL = $(LIBRARY)

LIBRARY := libMPNN.a
$(LIBRARY): $(OBJECTS)
	@echo Linking MPNN library...
	@rm -f $@
	@$(AR) $@ $^

# Cleanup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.PHONY: clean
clean:
	@echo Cleaning MPNN library...
	@rm -f $(OBJECTS) $(LIBRARY)

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
