
BUILDDIR?=./bin

include ./makefile.config


all:
	mkdir -p $(CURDIR)/../bin
	make -C Common all
	make -C HeptaneExtract all
	make -C HeptaneAnalysis all

theDoc:
	make -C Common doc
	make -C HeptaneExtract doc
	make -C HeptaneAnalysis doc

# cleanup
# -------
cleandoc:
	$(RM) -R Common/cfglib/generated-doc/html
	$(RM) -R HeptaneExtract/doc/generated-doc/html
	$(RM) -R HeptaneAnalysis/doc/generated-doc/html

clean: cleantheDoc
	find . -name '*.o' -print0 | xargs -0 $(RM) 
	find . -name '*~'  -print0 | xargs -0 $(RM)
	find . -name 'makefile.depends' -print0 | xargs -0 $(RM)

force: clean
	make -C Common force
	make -C HeptaneExtract force
	make -C HeptaneAnalysis force
