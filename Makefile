SCONS = scons -C code -j$(shell nproc || echo 1)

all: sneezy

# quick build
sneezy:
	$(SCONS)

# debugging build with runtime checks
debug:
	$(SCONS) asan=1 ubsan=1 debug=1 olevel=g

# production-optimized build
prod:
	$(SCONS) asan=1 optimize=1

test:
	$(SCONS) check

clean:
	$(SCONS) -c
	cd code && rm -rf config.log .sconf_temp .sconsign.dblite
	find -name '*.pyc' -o -name '__pycache__' -print0 -exec rm -r \{\} \;
