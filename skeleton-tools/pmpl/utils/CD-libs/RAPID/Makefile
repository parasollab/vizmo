# External Makefiles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

include ../../makefile_includes/Makefile.defaults

# Compiler Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

INCL := -I. -I..

# Object File Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

OBJECTS := RAPID.o build.o collide.o overlap.o

%.o: %.C
	@$(MAKE) -s check_platform
	@echo "Compiling $@..."
	@$(CXX) -c $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $< -o $@

# Library Configuration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# The library is the default target.
.DEFAULT_GOAL = $(LIBRARY)

LIBRARY := libRAPID.a
$(LIBRARY): $(OBJECTS)
	@echo Linking RAPID library...
	@rm -f $@
	@$(AR) $@ $^

# Extras ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

# Build the sample client included with the library.
sample_client: sample_client.C
	$(CXX) $(CXXFLAGS) $(OPTS) $(DEFS) $(INCL) $< -o $@ -L. -lRAPID -lm

# Cleanup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#

.PHONY: clean
clean:
	@echo Cleaning RAPID library...
	@rm -f $(LIBRARY) $(OBJECTS) sample_client

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
