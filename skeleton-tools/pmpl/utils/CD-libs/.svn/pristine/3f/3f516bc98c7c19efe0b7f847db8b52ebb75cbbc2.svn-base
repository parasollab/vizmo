# External Makefiles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

include ../../makefile_includes/Makefile.defaults

# Compiler Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

INCL := -I.

# Object File Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

OBJECTS := lib/PQP.o lib/BV.o lib/Build.o lib/TriDist.o

lib/%.o: src/%.cpp
	@$(MAKE) -s check_platform
	@echo "Compiling $@..."
	@$(CXX) -c $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $< -o $@

# Library Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# The library is the default target.
.DEFAULT_GOAL = $(LIBRARY)

LIBRARY := lib/libPQP.a
$(LIBRARY): $(OBJECTS)
	@echo Linking PQP library...
	@$(AR) $@ $^
	@cp src/*.h include/.

# Cleanup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.PHONY: clean
clean:
	@echo Cleaning PQP library...
	@rm -f $(LIBRARY) $(OBJECTS) include/*.h

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
