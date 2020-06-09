# Makefile for building a single configuration of the C interpreter. It expects
# variables to be passed in for:
#
# MODE         "debug" or "release".
# NAME         Name of the output executable (and object file directory).
# SOURCE_DIR   Directory where source files and headers are found.

CFLAGS := -Wall -Wextra -Werror -Wshadow -Wunused-function -Wunused-macros -fno-strict-aliasing
LFLAGS := -lm

DISABLE_HTTP := 0
ifeq ($(DISABLE_HTTP), 0)
  LFLAGS += -lcurl
endif

# Mode configuration.
ifeq ($(MODE),debug)
	CFLAGS += -O0 -DDEBUG -DDEBUG_STRESS_GC -DDEBUG_FINAL_MEM -g
	BUILD_DIR := build/debug
else ifeq ($(MODE),coverage)
    CFLAGS += -O0 -DDEBUG -DDEBUG_STRESS_GC -DDEBUG_FINAL_MEM -g -fprofile-arcs -ftest-coverage
    BUILD_DIR := build/coverage
else
	CFLAGS += -O3 -flto
	BUILD_DIR := build/release
endif

# Files.
HEADERS := $(wildcard $(SOURCE_DIR)/*.h) $(wildcard $(SOURCE_DIR)/datatypes/*.h) $(wildcard $(SOURCE_DIR)/optionals/*.h)
SOURCES := $(wildcard $(SOURCE_DIR)/*.c) $(wildcard $(SOURCE_DIR)/datatypes/*.c) $(wildcard $(SOURCE_DIR)/optionals/*.c)

ifeq ($(DISABLE_HTTP), 1)
    CFLAGS  += -DDISABLE_HTTP
    SOURCES := $(filter-out %http.c, $(SOURCES))
    HEADERS := $(filter-out %http.h, $(HEADERS))
endif

OBJECTS := $(addprefix $(BUILD_DIR)/$(NAME)/, $(notdir $(SOURCES:.c=.o)))
# Targets ---------------------------------------------------------------------

# Link the interpreter.
build/$(NAME): $(OBJECTS)
	@ printf "%8s %-40s %s\n" $(CC) $@ "$(CFLAGS)"
	@ mkdir -p build
	@ $(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

# Compile object files.
$(BUILD_DIR)/$(NAME)/%.o: $(SOURCE_DIR)/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@ mkdir -p $(BUILD_DIR)/$(NAME)
	@ $(CC) -c $(CFLAGS) -o $@ $<

# Compile object files.
$(BUILD_DIR)/$(NAME)/%.o: $(SOURCE_DIR)/datatypes/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@ mkdir -p $(BUILD_DIR)/$(NAME)
	@ $(CC) -c $(CFLAGS) -o $@ $<

# Compile object files.
$(BUILD_DIR)/$(NAME)/%.o: $(SOURCE_DIR)/optionals/%.c $(HEADERS)
	@ printf "%8s %-40s %s\n" $(CC) $< "$(CFLAGS)"
	@ mkdir -p $(BUILD_DIR)/$(NAME)
	@ $(CC) -c $(CFLAGS) -o $@ $<

.PHONY: default
