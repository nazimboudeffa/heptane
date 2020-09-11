
all:
	make -C cfglib all
	make -C ArchitectureDependent all
	make -C GlobalAttributes all
	make -C utl all
clean:
	make -C cfglib clean
	make -C ArchitectureDependent clean
	make -C GlobalAttributes clean
	make -C utl clean

doc:
	make -C cfglib doc
	make -C ArchitectureDependent doc
	make -C GlobalAttributes doc
	make -C utl doc

cleandoc:

force: clean all doc

