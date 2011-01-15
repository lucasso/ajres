#FLAGS_C=-I/usr/include/ -I/usr/local/include/boost-1_35/ -Wall
FLAGS_C=-Wall -Werror -ggdb  -gstabs  -gstabs+
FLAGS_ROOT_C= `root-config --cflags`
FLAGS_ROOT_L= `root-config --glibs`

srcs=longDataReader common graphs rmlpNet learningFactor
objs=$(patsubst %, obj/%.o, $(srcs))

all: bin/ajres bin/rmlpTest

bin/ajres: obj bin $(objs) obj/ajres.o
	g++ $(FLAGS_L) $(FLAGS_ROOT_L) $(objs) obj/ajres.o -o $@

bin/rmlpTest: obj bin $(objs) obj/rmlpTest.o
	g++ $(FLAGS_L) $(FLAGS_ROOT_L) $(objs) obj/rmlpTest.o -o $@

obj:
	mkdir obj

bin:
	mkdir bin

obj/graphs.o: graphs.cpp graphs.h
	g++ $(FLAGS_ROOT_C) $(FLAGS_C) -c $< -o $@

obj/%.o: %.cpp %.h
	g++ $(FLAGS_C) -c $< -o $@

obj/rmlpTest.o: tests/rmlpTest.cpp
	g++ $(FLAGS_C) -c $< -o $@

doc/konspekt.pdf: doc/konspekt.ps
	ps2pdf doc/konspekt.ps doc/konspekt.pdf

doc/konspekt.ps: konspekt.dvi
	dvips -f konspekt.dvi > doc/konspekt.ps
	
konspekt.dvi: konspekt.tex
	latex konspekt.tex
	latex konspekt.tex

clean:
	rm -rf obj/*.o bin/ajres bin/rmlpTest *~ 

cleandoc:
	rm -rf konspekt.dvi konspekt.log konspekt.aux konspekt.toc doc/konspekt.pdf doc/konspekt.ps doc/texput.log texput.log