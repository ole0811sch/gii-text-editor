CC = gcc
CFLAGS = -I../mockup -D MOCKUP -x c -std=c99 -Wall -pedantic \
		 -Wno-unknown-pragmas -g
BLDDIR = Build
OBJ = $(BLDDIR)/dyn_arrs.o $(BLDDIR)/editor.o $(BLDDIR)/font.o \
	  $(BLDDIR)/fxlib_mockup.o $(BLDDIR)/main.o $(BLDDIR)/util.o \
	  $(BLDDIR)/line.o line_utils.o
OBJ = $(addprefix $(BLDDIR)/, dyn_arrs.o editor.o font.o fxlib_mockup.o main.o \
	  util.o line.o line_utils.o commands.o)

.PHONY: all clean run build_dir

all: $(BLDDIR)/mockup.out
	cd ../mockup/ && $(MAKE)

clean: 
	rm -f -r $(BLDDIR)/
	cd ../mockup/ && $(MAKE) clean


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
	
$(BLDDIR)/fxlib_mockup.o: fxlib_mockup.cpp ../mockup/fxlib.h ../mockup/rfc.h \
	| build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/main.o: main.cpp | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/util.o: util.cpp | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

$(BLDDIR)/commands.o: commands.cpp commands.h | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build_dir: 
	mkdir -p $(BLDDIR)
