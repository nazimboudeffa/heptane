
INCLS+=-Isrc -Isrc/Generic -Isrc/SharedAttributes -Isrc/CacheAnalysis -Isrc/CacheAnalysis -Isrc/CodeLine -Isrc/DataAddressAnalysis -Isrc/DotPrint
INCLS+=-Isrc/DummyAnalysis -Isrc/HtmlPrint -Isrc/IPETAnalysis -Isrc/PipelineAnalysis -Isrc/SimplePrint


OBJS=obj/main.o obj/Config.o obj/CallGraph.o obj/Analysis.o obj/AnalysisHelper.o obj/Timer.o obj/Context.o obj/ContextHelper.o \
obj/CodeLine.o obj/CodeLineAttribute.o  obj/HtmlPrint.o \
obj/SimplePrint.o obj/DotPrint.o obj/Cache.o obj/ICacheAnalysis.o obj/DCacheAnalysis.o obj/CacheStatistics.o obj/IPETAnalysis.o obj/Solver.o obj/RegState.o obj/MIPSRegState.o \
obj/StackAnalysis.o obj/AddressAnalysis.o obj/MIPSAddressAnalysis.o obj/ARMAddressAnalysis.o \
obj/PipelineAnalysis.o obj/MIPSPipelineAnalysis.o obj/InstructionPipeline.o obj/ARMPipelineAnalysis.o obj/ARMRegState.o \
obj/StackInfoAttribute.o obj/DummyAnalysis.o 

vbin=../../bin/HeptaneAnalysis
all: $(vbin)

include ../makefile.common
include makefile.depends

# To build a new analysis
# - CFGLIBLIB (cfg management library)
# - Analysis.o (analysis template to be inherited from)
# - object file for your different analyses (here CacheAnalysis.o AddressAnalysis.o SimplePrint.o)
# - main.o (calls the different analyses)
$(vbin): $(CFGLIBDIR)/lib/libcfg.a $(OBJS) \
	$(CFGLIB) \
	$(ARCHDEP_DIR_OBJ)/MIPS.o\
	$(ARCHDEP_DIR_OBJ)/ARM.o\
	$(ARCHDEP_DIR_OBJ)/arch.o\
	$(ARCHDEP_DIR_OBJ)/InstructionType.o\
	$(ARCHDEP_DIR_OBJ)/InstructionFormat.o\
        $(ARCHDEP_DIR_OBJ)/DAAInstruction.o \
        $(ARCHDEP_DIR_OBJ)/DAAInstruction_MIPS.o \
        $(ARCHDEP_DIR_OBJ)/DAAInstruction_ARM.o \
	$(GLOB_ATTR_DIR_OBJ)/LoopTree.o\
	$(GLOB_ATTR_DIR_OBJ)/AddressAttribute.o\
	$(GLOB_ATTR_DIR_OBJ)/SymbolTableAttribute.o\
	$(GLOB_ATTR_DIR_OBJ)/ARMWordsAttribute.o \
	$(GLOB_ATTR_DIR_OBJ)/MetaInstructionAttribute.o \
	$(UTILITY_DIR_OBJ)/Logger.o $(UTILITY_DIR_OBJ)/Utl.o $(UTILITY_DIR_OBJ)/InstructionARM.o

	$(CXX) $^ $(LINKSFLAGS) -o $@


clean:
	$(RM) $(vbin) $(OBJS) doc/generated-doc/html/*.html


force: clean all doc

