# Top-level Makefile for fun-with-coroutines
# Requires: g++ 13+, libgtest-dev (e.g. sudo apt install libgtest-dev)

# ----- Compiler and flags -----
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -Werror -pthread \
            -fsanitize=address -fno-omit-frame-pointer
LDFLAGS  := -fsanitize=address -pthread

# ----- Directories -----
BUILD_DIR := build
BIN_DIR   := bin

# ----- Output -----
BIN      := $(BIN_DIR)/fun_with_coroutines
REPL_BIN := $(BIN_DIR)/calc_repl

# ----- Sources -----
COMMON_SRCS     := $(shell find common        -name '*.cpp' 2>/dev/null)
COROUTINES_SRCS := $(shell find coroutines    -name '*.cpp' 2>/dev/null | grep -v repl_main)
NOCORO_SRCS     := $(shell find no-coroutines -name '*.cpp' 2>/dev/null)
TEST_SRCS       := $(shell find tests         -name '*.cpp' 2>/dev/null)

SRCS := $(COMMON_SRCS) $(COROUTINES_SRCS) $(NOCORO_SRCS) $(TEST_SRCS)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# ----- Include paths -----
INCLUDES := -I.

# ----- GoogleTest -----
GTEST_LIBS := -lgtest_main -lgtest

# ----- Sources for the REPL binary -----
REPL_SRCS := coroutines/04-async-io/repl_main.cpp \
             common/pipe/Pipe.cpp \
             common/reactor/Reactor.cpp \
             common/event_loop/EventLoop.cpp \
             common/parser/InputBuffer.cpp \
             coroutines/04-async-io/calc/CalcServer.cpp \
             coroutines/04-async-io/calc/CalcClient.cpp \
             coroutines/04-async-io/run_async_io.cpp
REPL_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(REPL_SRCS))

# ----- .PHONY and all -----
.PHONY: all test clean

all: $(BIN) $(REPL_BIN)

test: $(BIN)
	$(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^ $(GTEST_LIBS)

$(REPL_BIN): $(REPL_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) -o $@ $^

# ----- Pattern rules and clean -----
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
