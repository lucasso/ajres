#FLAGS_C=-I/usr/include/ -I/usr/local/include/boost-1_35/ -Wall
FLAGS_C=-Wall -Werror
FLAGS_ROOT_C= `root-config --cflags`
FLAGS_ROOT_L= `root-config --glibs`

srcs=ajres longDataReader common graphs rmlpNet
objs=$(patsubst %, obj/%.o, $(srcs))

bin/ajres: obj bin $(objs)
	g++ $(FLAGS_L) $(FLAGS_ROOT_L) $(objs) -o $@

obj:
	mkdir obj

bin:
	mkdir bin

obj/graphs.o: graphs.cpp graphs.h
	g++ $(FLAGS_ROOT_C) $(FLAGS_C) -c $< -o $@

obj/%.o: %.cpp %.h
	g++ $(FLAGS_C) -c $< -o $@

doc/konspekt.pdf: doc/konspekt.ps
	ps2pdf doc/konspekt.ps doc/konspekt.pdf

doc/konspekt.ps: konspekt.dvi
	dvips -f konspekt.dvi > doc/konspekt.ps
	
konspekt.dvi: konspekt.tex
	latex konspekt.tex
	latex konspekt.tex

clean:
	rm -rf obj/*.o bin/ajres *~ 

cleandoc:
	rm -rf konspekt.dvi konspekt.log konspekt.aux konspekt.toc doc/konspekt.pdf doc/konspekt.ps doc/texput.log texput.log