CC = clang
CFLAGS = -I../mockup -D MOCKUP -Wall -pedantic -Wno-unknown-pragmas -g
OBJ = dyn_arrs.o editor.o font.o fxlib_mockup.o main.o util.o

.PHONY: all clean run

all: mockup.out
	cd ../mockup/ && $(MAKE)

run: 
	java -jar -ea ../mockup/build/mockup_gui.jar

clean: 
	rm -f $(OBJ) mockup.out
	cd ../mockup/ && $(MAKE) clean


mockup.out: $(OBJ)
	$(CC) $(CFLAGS) $^ -o mockup.out

dyn_arrs.o: dyn_arrs.cpp dyn_arrs.h dyn_arr.h
	$(CC) $(CFLAGS) -c $< -o $@

editor.o: editor.cpp editor.h
	$(CC) $(CFLAGS) -c $< -o $@

font.o: font.cpp font.h
	$(CC) $(CFLAGS) -c $< -o $@
	
fxlib_mockup.o: fxlib_mockup.cpp
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

util.o: util.cpp
	$(CC) $(CFLAGS) -c $< -o $@
