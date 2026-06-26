CXX := g++
CXXFLAGS := -std=c++20 -O2 -s -masm=intel -no-pie

SRC_DIR := src
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
NAMES := $(basename $(notdir $(SRCS)))

OUT64 := $(addprefix $(BUILD_DIR)/,$(addsuffix -64.out,$(NAMES)))
OUT32 := $(addprefix $(BUILD_DIR)/,$(addsuffix -32.out,$(NAMES)))

OUT32_MAIN := $(BUILD_DIR)/32-bit-main.out
OUT64_MAIN := $(BUILD_DIR)/64-bit-main.out

all: $(BUILD_DIR) $(OUT64) $(OUT32) $(OUT32_MAIN) $(OUT64_MAIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%-64.out: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -m64 $< -o $@

$(BUILD_DIR)/%-32.out: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -m32 $< -o $@

$(OUT32_MAIN): 32-bit-main.cpp
	$(CXX) $(CXXFLAGS) -m32 $< -o $@

$(OUT64_MAIN): 64-bit-main.cpp
	$(CXX) $(CXXFLAGS) -m64 $< -o $@

clean:
	rm -f $(OUT64) $(OUT32) $(OUT32_MAIN) $(OUT64_MAIN)
	rmdir $(BUILD_DIR)

.PHONY: all clean