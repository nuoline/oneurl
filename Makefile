LIBNAME =liboneurl.a
SRC_DIR=src

SOURCES = $(wildcard $(SRC_DIR)/*.cc)
OBJS = $(patsubst %.cc,%.o,$(SOURCES))

#complier information
CC=g++
INCPATH = -I./include -I./lib4/icu4c/include
COMOPT := -g -O2 -fPIC

all:$(OBJS)
	ar -r $(LIBNAME) $(OBJS)
	ranlib $(LIBNAME)
	cp $(LIBNAME) output/lib
	cp include/* output/include
	cp readme readme_zh output
	cd test && make && cd ..
	cp test/test test/test2 output/bin
%.o:%.cc
	$(CC) -c $(COMOPT) $(INCPATH) -o $@ $<

clean:
	rm -rf ./*.a
	rm -rf ./src/*.o
	rm -rf ./output/lib/*
	rm -rf ./output/include/*
	rm -rf ./output/bin/*
