##############################################################################
# See GNUmakefile.STAPL_README
###############################################################################
# this needs to be set before including STAPLdefaults

default: test

test: FORCE
	@cd pgraph/test    ; gmake -k test
	@cd parray/test    ; gmake -k test 
	@cd phash_map/test ; gmake -k test
	@cd pmatrix/test   ; gmake -k test 
	@cd plist/test     ; gmake -k test
	@cd pvector/test   ; gmake -k test

test-debug: DEBUG_FLAGS = -DSTAPL_DEBUG
test-debug: FORCE
	@cd pgraph/test    ; gmake DEBUG_FLAGS=$(DEBUG_FLAGS) -k test
	@cd parray/test    ; gmake DEBUG_FLAGS=$(DEBUG_FLAGS) -k test 
	@cd phash_map/test ; gmake DEBUG_FLAGS=$(DEBUG_FLAGS) -k test
	@cd pmatrix/test   ; gmake DEBUG_FLAGS=$(DEBUG_FLAGS) -k test 
	@cd plist/test     ; gmake DEBUG_FLAGS=$(DEBUG_FLAGS) -k test
	@cd pvector/test   ; gmake DEBUG_FLAGS=$(DEBUG_FLAGS) -k test

FORCE:

clean::
	@cd pgraph/test    ; gmake clean
	@cd parray/test    ; gmake clean
	@cd phash_map/test ; gmake clean
	@cd pmatrix/test   ; gmake clean
	@cd plist/test     ; gmake clean
	@cd pvector/test   ; gmake clean
# EOF

