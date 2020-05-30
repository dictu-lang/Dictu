BUILD_DIR := build

# Compile a debug build of dictu.
debug:
	@ $(MAKE) -f c.make NAME=dictu MODE=debug SOURCE_DIR=c
	@ cp build/dictu dictu # For convenience, copy the interpreter to the top level.

# Remove all build outputs and intermediate files.
clean:
	@ rm -rf $(BUILD_DIR)
	@ rm -rf gen

# Compile the C interpreter.
dictu:
	@ $(MAKE) -f c.make NAME=dictu MODE=release SOURCE_DIR=c
	@ cp build/dictu dictu # For convenience, copy the interpreter to the top level.

# Compile and run gcovr tool
coverage:
	@ rm -rf $(BUILD_DIR)
	@ $(MAKE) -f c.make NAME=dictu MODE=coverage SOURCE_DIR=c
	@ cp build/dictu dictu # For convenience, copy the interpreter to the top level.
	@ rm -rf coverage/*
	@ ./dictu tests/runTests.du
	@ gcovr -r . --html --html-details -o coverage/coverage.html -e "c/linenoise.*" -e "c/debug.*" -e "c/optionals/jsonBuilderLib.*" -e "c/optionals/jsonParseLib.*" -e "c/main.*"

.PHONY: clean dictu debug coverage
