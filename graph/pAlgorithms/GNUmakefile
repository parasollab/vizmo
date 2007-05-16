##############################################################################
# See GNUmakefile.STAPL_README
###############################################################################
# this needs to be set before including STAPLdefaults

default: test

test: FORCE
	@cd test    ; gmake -k test

test-debug: DEBUG_FLAGS = -DSTAPL_DEBUG
test-debug: FORCE
	@cd test    ; gmake DEBUG_FLAGS=$(DEBUG_FLAGS) -k test

FORCE:

clean::
	@cd test    ; gmake clean
# EOF

