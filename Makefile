SCONS = scons -C code -j$(shell nproc || echo 1)

all: sneezy

# quick build
sneezy:
	$(SCONS)

# debugging build with runtime checks
debug:
	$(SCONS) debug=1

# production-optimized build
prod:
	$(SCONS) shared=0 harden=1 optimize=1 lto=1

test:
	$(SCONS) check

clean:
	$(SCONS) -c
	cd code && rm -rf config.log .sconf_temp .sconsign.dblite
	find -name '*.pyc' -o -name '__pycache__' -print0 -exec rm -r \{\} \;
	cd lib && rm -rf roomdata corpses immortals rent account player txt/stats

