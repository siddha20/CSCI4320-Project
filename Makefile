MPICXX := mpicxx
DEBUG_FLAGS := -DNOT_AIMOS
LINK_FLAGS := -lcudart

NVCC := nvcc


BUILD_DIR := ./build
SRC_DIRS := ./src
BIN_DIR := ./bin

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(INC_DIRS:%=-I%)


.PHONY: all
all: $(BIN_DIR)/celeb $(BIN_DIR)/vote-gen $(BIN_DIR)/vote-algo $(BIN_DIR)/graph-gen

$(BIN_DIR)/graph-gen: $(BUILD_DIR)/src/graph/graph-gen.cpp.o $(BUILD_DIR)/src/graph/graph-gen.cu.o
	mkdir -p $(BIN_DIR)
	$(MPICXX) $^ -o  $(BIN_DIR)/$@ $(LINK_FLAGS)

$(BIN_DIR)/celeb: $(BUILD_DIR)/src/celeb/celeb.cpp.o
	mkdir -p $(BIN_DIR)
	$(MPICXX) $^ -o  $(BIN_DIR)/$@

$(BIN_DIR)/vote-gen: $(BUILD_DIR)/src/vote/vote-gen.cpp.o
	mkdir -p $(BIN_DIR)
	$(MPICXX) $^ -o $(BIN_DIR)/$@

$(BIN_DIR)/vote-algo: $(BUILD_DIR)/src/vote/vote-algo.cpp.o
	mkdir -p $(BIN_DIR)
	$(MPICXX) $^ -o $(BIN_DIR)/$@

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(MPICXX) $(DEBUG_FLAGS) $(INC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.cu.o: %.cu
	mkdir -p $(dir $@)
	$(NVCC) $(DEBUG_FLAGS) $(INC_FLAGS) -c $< -o $@

.SECONDARY: $(OBJS)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)