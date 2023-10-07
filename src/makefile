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

$(BLDDIR)/dyn_arrs.o: dyn_arrs.cpp dyn_arrs.h dyn_arr.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/editor.o: editor.cpp editor.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/line.o: line.cpp line.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/line_utils.o: line_utils.cpp line_utils.h line.h editor.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/font.o: font.cpp font.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@
	
$(BLDDIR)/fxlib_mockup.o: fxlib_mockup.cpp ../mockup/rfc.h \
	| build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/main.o: main.cpp | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

test_main:
	$(CC) $(CFLAGS) -c -D TEST_MODE main.cpp -o $(BLDDIR)/main.o

$(BLDDIR)/util.o: util.cpp | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/commands.o: commands.cpp commands.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

test_commands: commands.cpp
	$(CC) $(CFLAGS) -c -D TEST_MODE commands.cpp -o $(BLDDIR)/commands.o

$(BLDDIR)/clipboard.o: clipboard.cpp clipboard.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/editor_render.o: editor_render.cpp editor.h editor_render.h | \
	build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build_dir: 
	mkdir -p $(BLDDIR)
