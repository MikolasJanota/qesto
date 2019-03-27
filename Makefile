-include makeenv
PROGRAM_NAME=qesto

GENF=./generated/Options.cc ./generated/Options.hh\
     ./generated/DataStructures.hh\
     ./generated/LevelInfo.hh ./generated/LevelInfo.cc\
     ./generated/Groups.hh ./generated/Groups.cc\
     ./generated/Qesto.hh ./generated/Qesto.cc\
     ./generated/QestoGroups.hh ./generated/QestoGroups.cc\
     ./generated/GroupInversion.hh ./generated/GroupInversion.cc
CSRCS    = $(wildcard *.cc) $(wildcard ./generated/*.cc)
NSRCS    = $(wildcard *.nw)
COBJS    = $(CSRCS:.cc=.o)
DEPENDS = ${COBJS:.o=.d}
PDFS    = $(NSRCS:.nw=.pdf)
LIBD =
LIBS =
CXX?=g++

CCFLAGS+=-I.

ifdef PROF
CCFLAGS+=-fprofile-arcs -ftest-coverage
CCFLAGS+=-pg
CCFLAGS+=-g
LNFLAGS+=-fprofile-arcs
LNFLAGS+=-pg
LNFLAGS+=-g
endif

ifdef DBG
CCFLAGS+=-O0
CCFLAGS+=-g
CCFLAGS+=-DDBG
MINISAT_DIR=./minisat/build/debug/lib
MSAT=ld
else
CCFLAGS+=-DNDEBUG
CCFLAGS+=-O3
MSAT=lr
MINISAT_DIR=./minisat/build/release/lib
endif

LIBD+=-L$(MINISAT_DIR)
MINISAT_LIB=$(MINISAT_DIR)/libminisat.a

ifdef NOO
CCFLAGS+=-O0
endif

CCFLAGS += -Wall -Wextra -DBUILDDATE='"$(BUILDDATE)"' -DDISTDATE='"$(DISTDATE)"'
CCFLAGS+=-std=c++17
CCFLAGS+=-pedantic
CCFLAGS+=-MMD

LIBS+=-lz
CCFLAGS+=-I./generated


ifdef STATIC
CCFLAGS+=-static
LNFLAGS+=-static
endif

ifdef EXPERT # allow using giving options, without options the solver's fairly dumb
CCFLAGS+=-DEXPERT
endif

LIBS+=-lminisat
CCFLAGS+=-I./minisat

## Filter used to typeset C++
NW_FILTER=-filter C++.filter

.PHONY: m sources gl all doc

all:
	$(MAKE) sources
	$(MAKE) $(PROGRAM_NAME)

satmake:
	@cd ./minisat ; $(MAKE) CXX=$(CXX) $(MSAT)

$(PROGRAM_NAME):  $(COBJS) $(MINISAT_LIB)
	@echo Linking: $@
	$(CXX) -o $@ $(COBJS) $(LNFLAGS) $(LIBD) $(LIBS)

$(MINISAT_LIB): satmake

sources: $(GENF)

main.o: main.cc

%.o:	%.cc
	@echo Compiling: $@
	@$(CXX) $(CCFLAGS) -c -o $@ $<

%.o:	%.c Makefile
	@echo Compiling: $@
	@$(CXX) $(CCFLAGS) -c -o $@ $<

./generated/Options.hh ./generated/Options.cc: option_generation
	@echo Generating: $@
	mkdir -p generated
	./option_generation  >./generated/Options.hh 2>./generated/Options.cc

clean:
	@rm -f $(DEPENDS)
	@rm -f $(GENF)
	@rm -f $(PROGRAM_NAME) $(PROGRAM_NAME).exe $(COBJS)
	cd ./minisat/; $(MAKE) clean

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
