STAPL  = ../../../
INCLUDE  = -I./ -I../ -I$(STAPL)/include/stlport -I$(STAPL)/include
LCLIB    = -L$(STAPL)/lib $(STAPL)/lib/libstlport_gcc.a  -lrt
CFLAGS = -DDYNAMIC_GRAPH  -DQUIETGRAPH=1 -D_STL_PORT -DNOPREDECESSORS
OPT = -g

all: tstn1 tstn2 tstn3 tstn4 tsto1 tsto2 tsto3 tsto4
tstn1: ngraph_test1.cc
	g++ $(OPT) -o tstn1 $(CFLAGS) ngraph_test1.cc $(INCLUDE) $(LCLIB) 
tstn2: ngraph_test2.cc
	g++ $(OPT) -o tstn2 $(CFLAGS) ngraph_test2.cc $(INCLUDE) $(LCLIB) 
tstn3:ngraph_test3.cc
	g++ $(OPT) -o tstn3 $(CFLAGS) ngraph_test3.cc $(INCLUDE) $(LCLIB) 
tstn4:test_main_p1.cc
	g++ $(OPT) -o tstn4 $(CFLAGS) test_main_p1.cc $(INCLUDE) $(LCLIB) 
tsto1:ograph_test1.cc
	g++ $(OPT) -o tsto1 $(CFLAGS) ograph_test1.cc $(INCLUDE) $(LCLIB)	
tsto2:ograph_test2.cc
	g++ $(OPT) -o tsto2 $(CFLAGS) ograph_test2.cc $(INCLUDE) $(LCLIB)	
tsto3:ograph_test3.cc
	g++ $(OPT) -o tsto3 $(CFLAGS) ograph_test3.cc $(INCLUDE) $(LCLIB)	
tsto4:test_oldmain_p1.cc
	g++ $(OPT) -o tsto4  $(CFLAGS) test_oldmain_p1.cc $(INCLUDE) $(LCLIB)

main:main.cc
	g++ $(OPT) -o exe $(CFLAGS) main.cc $(INCLUDE) $(LCLIB)



#old stuff

all3:
	/usr/bin/g++ -o tstn1   -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD ngraph_test1.cc -I../
	/usr/bin/g++ -o tstn2   -DQUIETGRAPH=1  -D_EXPLICIT_USE_NAMESPACE_STD ngraph_test2.cc -I../
	/usr/bin/g++ -o tstn3   -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD ngraph_test3.cc -I../
	/usr/bin/g++ -o tstn4   -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD test_main_p1.cc -I../
	g++ -o tsto1   -DQUIETGRAPH=1 ograph_test1.cc -I../
	g++ -o tsto2   -DQUIETGRAPH=1 ograph_test2.cc -I../
	g++ -o tsto3   -DQUIETGRAPH=1 ograph_test3.cc -I../
	g++ -o tsto4   -DQUIETGRAPH=1 test_oldmain_p1.cc -I../
old:
	g++ -o oldexe -g  -DQUIETGRAPH=1 oldmain.cc
	g++ -o exe -g  main.cc
sgi:
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tstn1 ngraph_test1.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tstn2 ngraph_test2.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tstn3 ngraph_test3.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tstn4 test_main_p1.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tsto1 ograph_test1.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tsto2 ograph_test2.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tsto3 ograph_test3.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/home/igt2293/stapl/include -I/home/igt2293/stapl/include/stlport -o tsto4 test_oldmain_p1.cc -L/home/igt2293/stapl/lib -lstlport_mipspro -rpath /home/igt2293/stapl/lib 


sgi2:
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L   -DQUIETGRAPH=1 -o tstn1 ngraph_test1.cc -I../
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L   -DQUIETGRAPH=1 -o tstn2 ngraph_test2.cc -I../
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L   -DQUIETGRAPH=1 -o tstn3 ngraph_test3.cc -I../
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L   -DQUIETGRAPH=1 -o tstn4 test_main_p1.cc -I../
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L  -DQUIETGRAPH=1 -o tsto1 ograph_test1.cc -I../
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L  -DQUIETGRAPH=1 -o tsto2 ograph_test2.cc -I../
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L  -DQUIETGRAPH=1 -o tsto3 ograph_test3.cc -I../
	CC -O0  -ansi -LANG:std -mips4 -n32 -D_POSIX_C_SOURCE=199506L  -DQUIETGRAPH=1 -o tsto4 test_oldmain_p1.cc -I../

#kcc on umbrella
kcc:
	/usr/local/KAI/KCC_BASE/bin/KCC -o tstn1   -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD ngraph_test1.cc -I../ 
	/usr/local/KAI/KCC_BASE/bin/KCC -o tstn2   -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD ngraph_test2.cc -I../ 
	/usr/local/KAI/KCC_BASE/bin/KCC -o tstn3   -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD ngraph_test3.cc -I../ 
	/usr/local/KAI/KCC_BASE/bin/KCC -o tstn4   -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD test_main_p1.cc -I../ 
	g++ -o tsto1   -DQUIETGRAPH=1 ograph_test1.cc -I../
	g++ -o tsto2   -DQUIETGRAPH=1 ograph_test2.cc -I../
	g++ -o tsto3   -DQUIETGRAPH=1 ograph_test3.cc -I../
	g++ -o tsto4   -DQUIETGRAPH=1 test_oldmain_p1.cc -I../
#not fixed yet
kccstlp:
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tstn1 ngraph_test1.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tstn2 ngraph_test2.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tstn3 ngraph_test3.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tstn4 test_main_p1.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tsto1 ograph_test1.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tsto2 ograph_test2.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tsto3 ograph_test3.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 
	/usr/local/KAI/KCC_BASE/bin/KCC -D_POSIX_C_SOURCE=199506L -D_STL_PORT -DQUIETGRAPH=1 -I../ -I/research/latexo/stapl/gabrielt_boston/stapl/include -I/research/latexo/stapl/gabrielt_boston/stapl/include/stlport -o tsto4 test_oldmain_p1.cc -L/research/latexo/stapl/gabrielt_boston/stapl/lib -lstlport_mipspro -rpath /research/latexo/stapl/gabrielt_boston/stapl/lib 

acc:
	aCC -Wl,-N -AA  -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD -I../ -g -o tsto1 ngraph_test1.cc
	aCC -Wl,-N -AA  -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD -I../ -g -o tsto2 ngraph_test2.cc
	aCC -Wl,-N -AA  -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD -I../ -g -o tsto3 ngraph_test3.cc
	aCC -Wl,-N -AA  -DQUIETGRAPH=1 -D_EXPLICIT_USE_NAMESPACE_STD -I../ -g -o tsto4 test_main_p1.cc
	aCC -Wl,-N -AA -D_gplusplus -DQUIETGRAPH=1 -I../ -g -o tsto1 ograph_test1.cc
	aCC -Wl,-N -AA -D_gplusplus -DQUIETGRAPH=1 -I../ -g -o tsto2 ograph_test2.cc
	aCC -Wl,-N -AA -D_gplusplus -DQUIETGRAPH=1 -I../ -g -o tsto3 ograph_test3.cc
	aCC -Wl,-N -AA -D_gplusplus -DQUIETGRAPH=1 -I../ -g -o tsto4 test_oldmain_p1.cc

clean:
	rm -rf *.o exe oldexe core tstn1 tstn2 tstn3 tstn4 tsto1 tsto2 tsto3 tsto4
