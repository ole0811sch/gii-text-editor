CC = gcc
CFLAGS = -I../mockup/includeFX -I../mockup -D MOCKUP -x c -std=c99 -Wall -pedantic \
		 -Wno-unknown-pragmas -g
BLDDIR = Build
OBJ = $(addprefix $(BLDDIR)/, dyn_arrs.o editor.o font.o fxlib_mockup.o main.o \
	  util.o line.o line_utils.o commands.o clipboard.o editor_render.o)

.PHONY: all clean run build_dir test test_main test_commands

all: $(BLDDIR)/mockup.out
	cd ../mockup/ && $(MAKE)

clean: 
	rm -f -r $(BLDDIR)/
	cd ../mockup/ && $(MAKE) clean

test: $(BLDDIR)/test.out
	./$^

$(BLDDIR)/test.out: $(OBJ) test_main test_commands | build_dir
	$(CC) $(CFLAGS) -x none $(OBJ) -o $@

$(BLDDIR)/mockup.out: $(OBJ) | build_dir
	$(CC) $(CFLAGS) -x none $^ -o $@

$(OBJ): $(BLDDIR)/%.o: %.cpp | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

test_main:
	$(CC) $(CFLAGS) -c -D TEST_MODE main.cpp -o $(BLDDIR)/main.o

test_commands: commands.cpp
	$(CC) $(CFLAGS) -c -D TEST_MODE commands.cpp -o $(BLDDIR)/commands.o

build_dir: 
	mkdir -p $(BLDDIR)
