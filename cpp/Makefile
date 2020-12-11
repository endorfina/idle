idle: prereq
	@$(MAKE) -sC .. idle

%: prereq
	@$(MAKE) -sC .. $@

prereq:
	@test -f ../Makefile || ../configure.sh dev

.PHONY: idle prereq
