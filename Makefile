BUILD_DIR := build

default: book dictu jlox

# Build the site.
book:
	@ python3 util/build.py

# Compile a debug build of dictu.
debug:
	@ $(MAKE) -f util/c.make NAME=dictud MODE=debug SOURCE_DIR=c

# Run a local development server for the site that rebuilds automatically.
serve:
	@ python3 util/build.py --serve

# Run the tests for the final version of dictu.
test_dictu: debug
	@ python3 util/test.py dictu

# Run the tests for every chapter's version of dictu.
test_c: debug c_chapters
	@ python3 util/test.py c

# Run the tests for every chapter's version of dictu and jlox.
test_all: debug jlox c_chapters java_chapters
	@ python3 util/test.py all

# Remove all build outputs and intermediate files.
clean:
	@ rm -rf $(BUILD_DIR)
	@ rm -rf gen

# Compile the C interpreter.
dictu:
	@ $(MAKE) -f util/c.make NAME=dictu MODE=release SOURCE_DIR=c
	@ cp build/dictu dictu # For convenience, copy the interpreter to the top level.


split_chapters:
	@ python3 util/split_chapters.py

.PHONY: book c_chapters clean dictu debug default diffs java_chapters jlox serve split_chapters test test_all test_c test_java
