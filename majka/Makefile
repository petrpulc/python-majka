CXX=g++
LDFLAGS=
# -DIL2 and -DUTF determine input/output encoding (ISO-8859-2 and UTF-8)
CPPFLAGS=-fPIC -g -O2 --pedantic -Wall -Wextra -DIL2
CPPFLAGS=-fPIC -g -O2 --pedantic -Wall -Wextra -DUTF

all: majka libmajka.so perl

majka.o: majka.cc majka.h
	${CXX} ${CPPFLAGS} -c $< -o $@
majka_bin.o : majka_bin.cc majka.h
	${CXX} ${CPPFLAGS} -c $< -o $@
majka: majka_bin.o majka.o
	${CXX} ${CPPFLAGS} $^ ${LDFLAGS} -o $@

libmajka.so: majka.o
	rm -f $@
	${CXX} -shared -Wl,-soname,$@.0 -o $@.0.0.0 $^
	ln -s $@.0.0.0 $@.0
	ln -s $@.0 $@ 

clean: clean_perl
	rm -f majka.o majka_bin.o libmajka.so* majka

perl:
	swig -c++ -perl5 majka.i
	${CXX} -c ${CPPFLAGS} -DSWIG majka.cc majka_wrap.cxx `perl -MExtUtils::Embed -e ccopts`
	${CXX} -shared majka.o majka_wrap.o -o Majka.so

clean_perl:
	rm -f majka_wrap.* Majka.so Majka.pm
