CXX = g++
INC = -I../output/include
CXXFLAGS = $(INC) -g -Wall -O2
LIB = -L../output/lib/ -L../lib4/icu4c/lib -loneurl -licuuc -licudata  -rdynamic  -ldl
sources = test.cpp
sources2 = test2.cc

# makefile的缺省目标
target = $(basename $(sources))
target2 = $(basename $(sources2))
.PHONY : all
all : $(target) $(target2)

# 每个目标文件的依赖及生成的命令

test2 : test2.o
	$(CXX) $< -o $@ $(LIB)
test : test.o
	$(CXX) $< -o $@ $(LIB)

# 每个.o文件的依赖及生成的命令
%.o : %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)
PHONY : clean
clean : 
	-rm -rf $(target) $(target2) *.o
