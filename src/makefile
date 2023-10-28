CC = gcc
CFLAGS = -I../mockup/includeFX -I../mockup -D MOCKUP -x c -std=c99 -Wall -pedantic \
		 -Wno-unknown-pragmas -g
BLDDIR = Build
TESTBLDDIR = Build/test
TESTSRCDIR = test
# all %.o files coming from the src/ dir
OBJ_NAMES = $(addsuffix .o, $(basename $(shell echo *.cpp)))
# test/%.cpp files (without path)
TEST_DIR_ALL_SRC_NAMES = $(notdir $(shell echo $(TESTSRCDIR)/*.cpp))
# src/%.cpp files (without path)
SRC_DIR_SRC_NAMES = $(notdir $(shell echo *.cpp))
# test/%.cpp files that don't rely on a synonymous file in src/ (without path)
TEST_DIR_EXL_SRC_NAMES = $(filter-out $(SRC_DIR_SRC_NAMES), \
						 $(TEST_DIR_ALL_SRC_NAMES))
# test/%.cpp files that do rely on a synonymous file in src/ (without path)
TEST_DIR_NON_EXL_SRC_NAMES = $(filter $(SRC_DIR_SRC_NAMES), \
						 $(TEST_DIR_ALL_SRC_NAMES))
TEST_DIR_EXL_OBJ_NAMES = $(addsuffix .o, $(basename $(TEST_DIR_EXL_SRC_NAMES)))
TEST_DIR_NON_EXL_OBJ_NAMES = $(addsuffix .o, $(basename \
							 $(TEST_DIR_NON_EXL_SRC_NAMES)))
# .o files with src file in test/ that doesn't include src file in src/
TEST_DIR_EXL_OBJ = $(addprefix $(TESTBLDDIR)/, $(TEST_DIR_EXL_OBJ_NAMES))
# .o files with src file in test/ that includes src file in src/
TEST_DIR_NON_EXL_OBJ = $(addprefix $(TESTBLDDIR)/, \
					   $(TEST_DIR_NON_EXL_OBJ_NAMES))
# .o files with src file in test/
TEST_DIR_OBJ = $(TEST_DIR_EXL_OBJ) $(TEST_DIR_NON_EXL_OBJ)
# .o from src/%.cpp files with path (without test files)
OBJ = $(addprefix $(BLDDIR)/, $(OBJ_NAMES))
# all .o files with path that are identical for testing and mockup (except for
# that TEST_MODE macro)
TEST_OBJ_REGULAR = $(addprefix $(TESTBLDDIR)/, \
		   $(filter-out $(TEST_DIR_EXL_OBJ_NAMES) \
		   $(TEST_DIR_NON_EXL_OBJ_NAMES), $(OBJ_NAMES)))
# all %.o files needed for testing with correct paths
TEST_OBJ = $(TEST_OBJ_REGULAR) $(TEST_DIR_OBJ)

.PHONY: all clean run build_dir test make_debug

all: $(BLDDIR)/mockup.out
	cd ../mockup/ && $(MAKE)

clean: 
	rm -f -r $(BLDDIR)/
	cd ../mockup/ && $(MAKE) clean

test: $(TESTBLDDIR)/test.out
	./$^

make_debug:
	@echo "TEST_OBJ_REGULAR: $(TEST_OBJ_REGULAR)"
	@echo "TEST_DIR_EXL_OBJ_NAMES: $(TEST_DIR_EXL_OBJ_NAMES)"
	@echo "TEST_DIR_NON_EXL_OBJ_NAMES: $(TEST_DIR_NON_EXL_OBJ_NAMES)"

$(TESTBLDDIR)/test.out: $(TEST_OBJ) | build_dir
	$(CC) $(CFLAGS) -x none $(TEST_OBJ) -o $@

$(BLDDIR)/mockup.out: $(OBJ) | build_dir
	$(CC) $(CFLAGS) -x none $^ -o $@

$(TEST_OBJ_REGULAR): $(TESTBLDDIR)/%.o: %.cpp | build_dir
	$(CC) $(CFLAGS) -c -D TEST_MODE $< -o $@

$(TEST_DIR_EXL_OBJ): $(TESTBLDDIR)/%.o: $(TESTSRCDIR)/%.cpp | build_dir
	$(CC) $(CFLAGS) -c -D TEST_MODE $< -o $@

$(TEST_DIR_NON_EXL_OBJ): $(TESTBLDDIR)/%.o: $(TESTSRCDIR)/%.cpp %.cpp \
	| build_dir
	$(CC) $(CFLAGS) -c -D TEST_MODE $< -o $@

$(OBJ): $(BLDDIR)/%.o: %.cpp | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build_dir: 
	mkdir -p $(BLDDIR)
	mkdir -p $(TESTBLDDIR)
