TEMP_DIR=build
VER=0.1

all: clean_out shared static
debug: clean_out shared_debug static_debug

shared:
	mkdir -p $(TEMP_DIR)/out
	gcc -Wall -Werror -fpic -o $(TEMP_DIR)/html.o -c html.c
	gcc -shared -o $(TEMP_DIR)/out/libhtml_$(VER).so $(TEMP_DIR)/html.o
static:
	mkdir -p $(TEMP_DIR)/out
	gcc -Wall -Werror -o $(TEMP_DIR)/libhtml.o -c html.c
	ar rcs $(TEMP_DIR)/out/libhtml_$(VER).a $(TEMP_DIR)/html.o
shared_debug:
	mkdir -p $(TEMP_DIR)/out
	gcc -g -Wall -Werror -fpic -DDEBUG -o $(TEMP_DIR)/html.o -c html.c
	gcc -shared -o $(TEMP_DIR)/out/libhtml_$(VER).so $(TEMP_DIR)/html.o
static_debug:
	mkdir -p $(TEMP_DIR)/out
	gcc -g -Wall -Werror -DDEBUG -o $(TEMP_DIR)/libhtml.o -c html.c
	ar rcs $(TEMP_DIR)/out/libhtml_$(VER).a $(TEMP_DIR)/html.o
clean_out:
	rm -rf build/out
clean_build:
	rm -rf build
