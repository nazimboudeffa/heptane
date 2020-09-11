
INCLS+=-Isrc 
OBJS=obj/HeptaneExtract.o obj/ConfigExtract.o obj/dominatorData.o obj/dominatorAnalysis.o obj/loopAnalysis.o obj/Annotations.o  obj/switchAnalysis.o

vbin=../../bin/HeptaneExtract
all: $(vbin)

include ../makefile.common
include makefile.depends


$(vbin): $(ARCHDEP_DIR_OBJ)/MIPS.o $(ARCHDEP_DIR_OBJ)/ARM.o $(ARCHDEP_DIR_OBJ)/arch.o $(ARCHDEP_DIR_OBJ)/InstructionFormat.o $(ARCHDEP_DIR_OBJ)/InstructionType.o \
        $(ARCHDEP_DIR_OBJ)/DAAInstruction.o $(ARCHDEP_DIR_OBJ)/DAAInstruction_MIPS.o  $(ARCHDEP_DIR_OBJ)/DAAInstruction_ARM.o \
	$(OBJS) $(CFGLIBDIR)/lib/libcfg.a\
	$(UTILITY_DIR_OBJ)/Logger.o $(UTILITY_DIR_OBJ)/Utl.o \
        $(GLOB_ATTR_DIR_OBJ)/AddressAttribute.o $(GLOB_ATTR_DIR_OBJ)/LoopTree.o  $(GLOB_ATTR_DIR_OBJ)/SymbolTableAttribute.o $(GLOB_ATTR_DIR_OBJ)/ARMWordsAttribute.o $(GLOB_ATTR_DIR_OBJ)/MetaInstructionAttribute.o
	$(CXX) $^ $(LINKSFLAGS) -o $@

clean:
	$(RM) $(vbin) $(OBJS) doc/generated-doc/html/*.html

force: clean all doc

