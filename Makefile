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


.PHONY: clean dictu debug
