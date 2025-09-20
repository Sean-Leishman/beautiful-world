DEFAULT_PRESET ?= default
BUILD_DIR ?= build

# Clang tools (adjust version as needed)
CLANG_FORMAT ?= clang-format-21
CLANG_TIDY ?= clang-tidy-21

# Source directories
SRC_DIRS := src include
SRC_FILES := $(shell find $(SRC_DIRS) -name '*.cpp' -o -name '*.hpp' -o -name '*.h' 2>/dev/null)
MAIN_FILES := main.cpp

# Colors for output
GREEN := \033[0;32m
YELLOW := \033[1;33m
RED := \033[0;31m
NC := \033[0m # No Color

.PHONY: help configure build clean list-presets debug release release-lto test install
.PHONY: format format-check tidy tidy-fix lint
.PHONY: example-binary example-phong example-pathtracer examples

# Default target
all: configure build

help:
	@echo "$(GREEN)Available targets:$(NC)"
	@echo "  help         - Show this help message"
	@echo "  list-presets - List all available CMake presets"
	@echo "  configure    - Configure using default preset ($(DEFAULT_PRESET))"
	@echo "  build        - Build using default preset"
	@echo "  debug        - Configure and build debug version (with sanitizers)"
	@echo "  release      - Configure and build release version (optimized)"
	@echo "  release-lto  - Configure and build release with LTO (max optimization)"
	@echo "  clean        - Clean build directory"
	@echo "  test         - Run tests"
	@echo "  install      - Install the project"
	@echo ""
	@echo "$(GREEN)Code quality targets:$(NC)"
	@echo "  format       - Format all source files with clang-format"
	@echo "  format-check - Check formatting without modifying files"
	@echo "  tidy         - Run clang-tidy static analysis"
	@echo "  tidy-fix     - Run clang-tidy and apply fixes"
	@echo "  lint         - Run both format-check and tidy"
	@echo ""
	@echo "$(GREEN)Example targets:$(NC)"
	@echo "  example-binary     - Render simple binary example"
	@echo "  example-phong      - Render phong shading example"
	@echo "  example-pathtracer - Render pathtracer Cornell box"
	@echo "  examples           - Render all examples"
	@echo ""
	@echo "$(YELLOW)Environment variables:$(NC)"
	@echo "  DEFAULT_PRESET - Set default preset (current: $(DEFAULT_PRESET))"
	@echo "  BUILD_DIR      - Set build directory (current: $(BUILD_DIR))"

list-presets:
	@echo "$(GREEN)Available CMake presets:$(NC)"
	@cmake --list-presets=all 2>/dev/null || echo "No presets found or CMake version < 3.20"

configure:
	@echo "$(GREEN)Configuring with preset: $(DEFAULT_PRESET)$(NC)"
	@cmake --preset $(DEFAULT_PRESET)

build: configure
	@echo "$(GREEN)Building with preset: $(DEFAULT_PRESET)$(NC)"
	@cmake --build --preset $(DEFAULT_PRESET)

debug:
	@echo "$(GREEN)Configuring and building debug version$(NC)"
	@cmake --preset debug
	@cmake --build --preset debug

release:
	@echo "$(GREEN)Configuring and building release version$(NC)"
	@cmake --preset release
	@cmake --build --preset release

release-lto:
	@echo "$(GREEN)Configuring and building release with LTO$(NC)"
	@cmake --preset release-lto
	@cmake --build --preset release-lto

clean:
	@echo "$(YELLOW)Cleaning build directory$(NC)"
	@rm -rf $(BUILD_DIR)

# Code formatting with clang-format
format:
	@echo "$(GREEN)Formatting source files with $(CLANG_FORMAT)$(NC)"
	@$(CLANG_FORMAT) -i $(SRC_FILES) $(MAIN_FILES)
	@echo "$(GREEN)Done formatting$(NC)"

format-check:
	@echo "$(GREEN)Checking source file formatting$(NC)"
	@$(CLANG_FORMAT) --dry-run --Werror $(SRC_FILES) $(MAIN_FILES) && \
		echo "$(GREEN)All files properly formatted$(NC)" || \
		(echo "$(RED)Some files need formatting. Run 'make format' to fix.$(NC)" && exit 1)

# Static analysis with clang-tidy
tidy: configure
	@echo "$(GREEN)Running $(CLANG_TIDY) static analysis$(NC)"
	@$(CLANG_TIDY) -p $(BUILD_DIR)/$(DEFAULT_PRESET) $(SRC_FILES) $(MAIN_FILES) -- \
		-I include -std=c++20
	@echo "$(GREEN)Done with static analysis$(NC)"

tidy-fix: configure
	@echo "$(GREEN)Running $(CLANG_TIDY) with auto-fix$(NC)"
	@$(CLANG_TIDY) -p $(BUILD_DIR)/$(DEFAULT_PRESET) --fix $(SRC_FILES) $(MAIN_FILES) -- \
		-I include -std=c++20
	@echo "$(GREEN)Done with static analysis and fixes$(NC)"

# Combined lint target
lint: format-check tidy
	@echo "$(GREEN)All lint checks passed$(NC)"

# Example output directory
OUTPUT_DIR := output

# Example targets
example-binary: build
	@mkdir -p $(OUTPUT_DIR)
	@echo "$(GREEN)Rendering binary example...$(NC)"
	./$(BUILD_DIR)/$(DEFAULT_PRESET)/main /examples/simple_binary.json /$(OUTPUT_DIR)/binary_output.ppm

example-phong: build
	@mkdir -p $(OUTPUT_DIR)
	@echo "$(GREEN)Rendering phong example...$(NC)"
	./$(BUILD_DIR)/$(DEFAULT_PRESET)/main /examples/phong_spheres.json /$(OUTPUT_DIR)/phong_output.ppm

example-pathtracer: build
	@mkdir -p $(OUTPUT_DIR)
	@echo "$(GREEN)Rendering pathtracer example...$(NC)"
	./$(BUILD_DIR)/$(DEFAULT_PRESET)/main /examples/pathtracer_cornell.json /$(OUTPUT_DIR)/pathtracer_output.ppm

examples: example-binary example-phong example-pathtracer
	@echo "$(GREEN)All examples rendered to $(OUTPUT_DIR)/$(NC)"
