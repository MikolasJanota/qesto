GENF=./generated/Options.cc ./generated/Options.hh\
     ./generated/DataStructures.hh\
     ./generated/LevelInfo.hh ./generated/LevelInfo.cc\
     ./generated/Groups.hh ./generated/Groups.cc\
     ./generated/Qesto.hh ./generated/Qesto.cc\
     ./generated/QestoGroups.hh ./generated/QestoGroups.cc\
     ./generated/GroupInversion.hh ./generated/GroupInversion.cc
CSRCS    = $(wildcard *.cc) $(wildcard ./generated/*.cc)
NSRCS    = $(wildcard *.nw)
YACC     = $(wildcard *.y)
LEX      = $(wildcard *.l)
COBJS    = $(CSRCS:.cc=.o) $(YACC:.y=.tab.o) $(LEX:.l=.o)
DEPENDS = ${COBJS:.o=.d}
PDFS    = $(NSRCS:.nw=.pdf)
LIBD =
LIBS =
CXX?=g++
CC=g++

CFLAGS+=-I.

ifdef PROF
CFLAGS+=-fprofile-arcs -ftest-coverage
CFLAGS+=-pg
CFLAGS+=-g
LNFLAGS+=-fprofile-arcs
LNFLAGS+=-pg
LNFLAGS+=-g
endif

ifdef DBG
CFLAGS+=-O0
CFLAGS+=-ggdb
CFLAGS+=-DDBG
MSAT=libd
else
CFLAGS+=-DNDEBUG
CFLAGS+=-O3
MSAT=libr
endif

ifdef NOO
CFLAGS+=-O0
endif

CFLAGS += -Wall -DBUILDDATE='"$(BUILDDATE)"' -DDISTDATE='"$(DISTDATE)"'
CFLAGS += -DCHANGESET='"$(CHANGESET)"' -DRELDATE='"$(RELDATE)"'
CFLAGS+=-D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -Wno-parentheses -Wno-deprecated -D _MSC_VER
#CFLAGS+=-std=c++11
CFLAGS+=-std=c++0x
CFLAGS+=-MMD
LIBS+=-lz
CFLAGS+=-I./generated

ifdef USE_GL
EXTERNAL+=gl
CFLAGS+=-I./glucose-3.0/
LIBD+=-L./glucose-3.0/core
LIBD+=-L./glucose-3.0/simp
LIBS+=-lglucose
LIBS+=-lglucose_simp
CFLAGS+=-DUSE_GL
else
EXTERNAL+=m
LIBS+=-lminisat
LIBS+=-lminisat_simp
LIBD+=-L./minisat/core
LIBD+=-L./minisat/simp
CFLAGS+=-I./minisat/
endif

ifdef STATIC
CFLAGS+=-static
LNFLAGS+=-static
endif

ifdef USE_SIMP
CFLAGS+=-DUSE_SIMP
endif

ifdef NON_INCR
CFLAGS+=-DNON_INCR
endif

## Filter used to typeset C++
NW_FILTER=-filter C++.filter

.PHONY: m sources gl all doc

all:
	make sources
#make doc
	make qesto

qesto:  $(COBJS) $(EXTERNAL)
	@echo Linking: $@
	$(CXX) -o $@ $(COBJS) $(LNFLAGS) $(LIBD) $(LIBS)

m:
	export MROOT=`pwd`/minisat ; cd ./minisat/core; make CXX=$(CXX) LIB=minisat $(MSAT)
	export MROOT=`pwd`/minisat ; cd ./minisat/simp; make CXX=$(CXX) LIB=minisat_simp $(MSAT)

gl:
	export MROOT=`pwd`/glucose-3.0 ; cd ./glucose-3.0/core; make CXX=$(CXX) LIB=glucose $(MSAT)
	export MROOT=`pwd`/glucose-3.0 ; cd ./glucose-3.0/simp; make CXX=$(CXX) LIB=glucose_simp $(MSAT)

sources: $(GENF)

main.o: main.cc

%.o:	%.cc
	@echo Compiling: $@
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.o:	%.c Makefile
	@echo Compiling: $@
	@$(CXX) $(CFLAGS) -c -o $@ $<

%.tab.h %.tab.c: %.y
	@echo Generating: $*
	@bison $<

%.c:	%.l Makefile
	@echo Generating: $*
	@flex -o$@ $<

./generated/Options.hh ./generated/Options.cc: option_generation
	@echo Generating: $@
	mkdir -p generated
	./option_generation  >./generated/Options.hh 2>./generated/Options.cc

clean:
	@rm -f $(DEPENDS)
	@rm -f $(GENF)
	@rm -f qesto qesto.exe $(COBJS) *.tab.[ch]
	@export MROOT=`pwd`/minisat ; cd ./minisat/core; make CXX=$(CXX) clean
	@export MROOT=`pwd`/minisat ; cd ./minisat/simp; make CXX=$(CXX) clean

generated/%.hh: %.nw
	notangle -L -R$(notdir $@) $< >$@

generated/%.cc: %.nw
	notangle -L -R$(notdir $@) $< >$@

doc: $(PDFS)

%.pdf:  generated/%.tex
	@latexmk -pdf $<

generated/%.tex:  %.nw texheader.tex Makefile
	@noweave -x $(NW_FILTER) $< | sed 's/\\begin{document}/\\input{texheader.tex}\\begin{document}/' >$@

-include ${DEPENDS}
