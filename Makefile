# SneezyMUD Build System
#
# Usage:
#   make              - Build with default preset (dev-clang)
#   make run          - Run with sanitizer options
#   make PRESET=dev-gcc build  - Build with specific preset
#
# Available presets: dev-gcc, dev-clang, release-gcc, release-clang

PRESET ?= dev-clang
BUILD_DIR := build/$(PRESET)
BINARY := code/sneezy

# Sanitizer options for development runs
export ASAN_OPTIONS := strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1
export UBSAN_OPTIONS := print_stacktrace=1:halt_on_error=1

.PHONY: all build configure clean clean-all rebuild run debug verify format help
.PHONY: build-profile ninja-trace

all: build

# Configure the build directory using CMake preset
configure:
	cmake --preset $(PRESET)

# Build the project (configures if needed)
build:
	cmake --preset $(PRESET)
	cmake --build --preset $(PRESET)

# Clean the build directory for current preset
clean:
	rm -rf $(BUILD_DIR)

# Clean all build directories
clean-all:
	rm -rf build

# Clean and rebuild
rebuild: clean build

# Run sneezy with strict sanitizer options
run: build
	./$(BINARY)

# Run sneezy under gdb
debug: build
	gdb -ex run ./$(BINARY)

# Verify the game boots correctly (requires prior build)
verify:
	./scripts/verify_boot.sh

# Format C++ source files (all files, or specific file if FILE is set)
# Edits in-place using the rules in the .clang-format file in the root folder
format:
ifdef FILE
	clang-format -i --verbose $(FILE)
else
	find code/code/ -type f \( -iname "*.h" -o -iname "*.cc" \) -exec clang-format -i --verbose {} \;
endif

# Profile the build using ClangBuildAnalyzer (requires Clang preset)
build-profile:
	./scripts/build-profile.sh $(PRESET)

# Generate Ninja build trace for visualization
ninja-trace:
	./scripts/ninja-trace.sh $(PRESET)

help:
	@echo "SneezyMUD Build System"
	@echo ""
	@echo "Targets:"
	@echo "  build         - Configure and build (default)"
	@echo "  configure     - Configure only (no build)"
	@echo "  clean         - Remove build directory for current preset"
	@echo "  clean-all     - Remove all build directories"
	@echo "  rebuild       - Clean and rebuild current preset"
	@echo "  run           - Build and run with sanitizer options"
	@echo "  debug         - Build and run under gdb"
	@echo "  verify        - Verify game boots correctly (requires build)"
	@echo "  format        - Run clang-format (all files, or FILE=path)"
	@echo "  build-profile - Profile build with ClangBuildAnalyzer"
	@echo "  ninja-trace   - Generate Ninja build trace"
	@echo ""
	@echo "Variables:"
	@echo "  PRESET        - CMake preset (default: dev-clang)"
	@echo "  FILE          - Specific file for format target (optional)"
	@echo ""
	@echo "Available presets:"
	@echo "  dev-gcc       - Debug build with GCC, ASan + UBSan"
	@echo "  dev-clang     - Debug build with Clang, ASan + UBSan"
	@echo "  release-gcc   - Release build with GCC, ASan + UBSan + LTO"
	@echo "  release-clang - Release build with Clang, ASan + UBSan + ThinLTO"
	@echo ""
	@echo "Examples:"
	@echo "  make                          # Build with dev-clang"
	@echo "  make PRESET=dev-gcc           # Build with dev-gcc"
	@echo "  make run                      # Build and run"
	@echo "  make rebuild                  # Clean and rebuild"
	@echo "  make format FILE=code/code/foo.cc  # Format one file"
