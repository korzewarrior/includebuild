# IncludeBuild Makefile

PREFIX ?= /usr/local
VERSION = 1.0.0

.PHONY: all install uninstall test examples clean docs dist

all: examples

# Install IncludeBuild to system include directory
install:
	@echo "Installing IncludeBuild $(VERSION) to $(PREFIX)/include"
	@mkdir -p $(PREFIX)/include
	@cp src/build.h $(PREFIX)/include/
	@echo "IncludeBuild installed successfully"

# Uninstall IncludeBuild from system
uninstall:
	@echo "Uninstalling IncludeBuild from $(PREFIX)/include"
	@rm -f $(PREFIX)/include/build.h
	@echo "IncludeBuild uninstalled successfully"

# Run tests
test:
	@echo "Running IncludeBuild tests..."
	@mkdir -p build/tests
	@for test in tests/*.c; do \
		echo "Building and running $$test..."; \
		gcc -o build/tests/`basename $$test .c` $$test; \
		build/tests/`basename $$test .c`; \
	done
	@echo "All tests completed successfully"

# Build examples
examples:
	@echo "Building examples..."
	@mkdir -p build/examples
	@# Basic example
	@if [ -d examples/basic ]; then \
		cd examples/basic && gcc -o ../../build/examples/basic_main main.c && ../../build/examples/basic_main; \
	fi
	@# Multi-file example
	@if [ -d examples/multi-file ]; then \
		cd examples/multi-file && gcc -o ../../build/examples/multi_file_main main.c && ../../build/examples/multi_file_main; \
	fi
	@# Library example
	@if [ -d examples/library ]; then \
		cd examples/library && gcc -o ../../build/examples/library_main main.c && ../../build/examples/library_main; \
	fi
	@# Raylib Pong example (just build the builder, not the actual game as it requires raylib)
	@if [ -d examples/raylib-pong ]; then \
		cd examples/raylib-pong && gcc -o ../../build/examples/raylib_pong_main main.c; \
	fi
	@echo "Examples built successfully"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build
	@find examples -name "build" -type d -exec rm -rf {} \; 2>/dev/null || true
	@echo "Clean complete"

# Generate documentation
docs:
	@echo "Documentation generation is not yet implemented"
	@echo "For now, please refer to the README.md and docs/*.md files"

# Create distribution package
dist:
	@echo "Creating distribution package..."
	@mkdir -p dist
	@tar -czf dist/includebuild-$(VERSION).tar.gz LICENSE README.md Makefile src/ examples/ docs/
	@echo "Created dist/includebuild-$(VERSION).tar.gz" 