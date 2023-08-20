CC = clang
CFLAGS = -Wall -pedantic -std=c99 -I
SRC_DIR = ../src/ 
BUILD_DIR = Build/

.PHONY: all clean

all: mockup.out
	
mockup.out: $(BUILD_DIR)dyn_arrs.o $(BUILD_DIR)editor.o $(BUILD_DIR)font.o $(BUILD_DIR)main.o $(BUILD_DIR)fxlib_mockup.o $(BUILD_DIR)util.o

$(BUILD_DIR)dyn_arrs.o: $(SRC_DIR)dyn_arrs.cpp $(SRC_DIR)dyn_arrs.h $(SRC_DIR)dyn_arr.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)dyn_arrs.cpp -o $(BUILD_DIR)dyn_arrs.o

$(BUILD_DIR)editor.o: $(SRC_DIR)editor.cpp $(SRC_DIR)dyn_arrs.h $(SRC_DIR)dyn_arr.h $(SRC_DIR)fxlib.h $(SRC_DIR)keybios.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)editor.cpp -o $(BUILD_DIR)editor.o

$(BUILD_DIR)font.o: $(SRC_DIR)font.cpp $(SRC_DIR)font.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)font.cpp -o $(BUILD_DIR)font.o

$(BUILD_DIR)main.o: $(SRC_DIR)main.cpp $(SRC_DIR)main.h $(SRC_DIR)editor.h $(SRC_DIR)util.h $(SRC_DIR)fxlib.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)main.cpp -o $(BUILD_DIR)main.o

$(BUILD_DIR)fxlib_mockup.o: $(SRC_DIR)fxlib_mockup.cpp $(SRC_DIR)fxlib_mockup.h $(SRC_DIR)keybios.h
	$(CC) -c $(CFLAGS) fxlib_mockup.cpp -o $(BUILD_DIR)fxlib_mockup.o

$(BUILD_DIR)util.o: $(SRC_DIR)util.cpp $(SRC_DIR)util.h $(SRC_DIR)main.h $(SRC_DIR)fxlib.h
	$(CC) -c $(CFLAGS) $(SRC_DIR)util.cpp -o $(BUILD_DIR)util.o
