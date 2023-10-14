CC = gcc
CFLAGS = -I../mockup/includeFX -I../mockup -D MOCKUP -x c -std=c99 -Wall -pedantic \
		 -Wno-unknown-pragmas -g
BLDDIR = Build
TESTBLDDIR = Build/test
TESTSRCDIR = test
# all %.o files coming from the src/ dir
OBJ_NAMES = $(addsuffix .o, $(basename $(shell echo *.cpp)))
# test/%.cpp files without path
TEST_DIR_SRC_NAMES = $(notdir $(shell echo $(TESTSRCDIR)/*.cpp))
TEST_DIR_OBJ_NAMES = $(addsuffix .o, $(basename $(TEST_DIR_SRC_NAMES)))
TEST_DIR_OBJ = $(addprefix $(TESTBLDDIR)/, $(TEST_DIR_OBJ_NAMES))
# %.o files with path (without test files)
OBJ = $(addprefix $(BLDDIR)/, $(OBJ_NAMES))
# all .o files with path that are identical for testing and mockup
TEST_OBJ_REGULAR = $(addprefix $(TESTBLDDIR)/, \
		   $(filter-out $(TEST_DIR_OBJ_NAMES), $(OBJ_NAMES)))
# all %.o files needed for testing with correct paths
TEST_OBJ = $(TEST_OBJ_REGULAR) $(TEST_DIR_OBJ)

.PHONY: all clean run build_dir test make_debug

all: $(BLDDIR)/mockup.out
	cd ../mockup/ && $(MAKE)

clean: 
	rm -f -r $(BLDDIR)/
	cd ../mockup/ && $(MAKE) clean

test: $(TESTBLDDIR)/test.out
	$(eval TEST_RESULT = $(shell ./$^ > /dev/null; echo $$?))
	@printf "\nTest suite exited with code $(TEST_RESULT)\n\n"

make_debug:
	@echo "TEST_OBJ_REGULAR: $(TEST_OBJ_REGULAR)"
	@echo "TEST_DIR_OBJ: $(TEST_DIR_OBJ)"
	@echo "TEST_OBJ: $(TEST_OBJ)"
	@echo "TESTBLDDIR: $(TESTBLDDIR)"
	@echo "TEST_DIR_OBJ_NAMES: $(TEST_DIR_OBJ_NAMES)"

$(TESTBLDDIR)/test.out: $(TEST_OBJ) | build_dir
	$(CC) $(CFLAGS) -x none $(TEST_OBJ) -o $@

$(BLDDIR)/mockup.out: $(OBJ) | build_dir
	$(CC) $(CFLAGS) -x none $^ -o $@

$(TEST_OBJ_REGULAR): $(TESTBLDDIR)/%.o: %.cpp | build_dir
	$(CC) $(CFLAGS) -c -D TEST_MODE $< -o $@

$(TEST_DIR_OBJ): $(TESTBLDDIR)/%.o: $(TESTSRCDIR)/%.cpp | build_dir
	$(CC) $(CFLAGS) -c -D TEST_MODE $< -o $@

$(OBJ): $(BLDDIR)/%.o: %.cpp | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build_dir: 
	mkdir -p $(BLDDIR)
	mkdir -p $(TESTBLDDIR)
