
# compilateur et options de compilation (portable macOS)
BUILDDIR?=./build_dir
DESTDIR?=$(CURDIR)/../../cfglib_install

# libxml++ and dependencies
INCLS+=-I$(XML2) -I../utl/src

# la bibliothèque cfglib 
INCLS+=-I./include

CFGLIB_OBJ= obj/Factory.o obj/Attributed.o obj/SerialisableAttributes.o obj/XmlExtra.o obj/Handle.o \
   obj/Edge.o obj/Instruction.o obj/Node.o obj/Loop.o obj/Cfg.o obj/Program.o obj/PointerAttributes.o obj/CloneHandle.o

INCLUDESRC_DIRS=include
#EXTERNALINCLUDESRC_DIRS=external_lib/

OBJS=$(CFGLIB_OBJ)

include ../makefile.common

LIBRARIES=$(BUILDDIR)/libcfg.a
install: $(LIBRARIES) $(DESTDIR)/lib/libcfg.a $(DESTDIR)/doc/index.html

# install: libraries
$(DESTDIR)/lib/libcfg.a: $(LIBRARIES)
	mkdir -p $(DESTDIR)/lib $(DESTDIR)/include ;\
	cp -p $(LIBRARIES) $(DESTDIR)/lib ; \
	for dir in $(INCLUDESRC_DIRS) ; do \
		mkdir -p $(DESTDIR)/$$dir ; cp -p $$dir/*.h $(DESTDIR)/$$dir/ ; \
	done
	for dir in $(EXTERNALINCLUDESRC_DIRS) ; do \
		mkdir -p $(DESTDIR)/$$dir ; cp -p $$dir/*.h $(DESTDIR)/$$dir/ ; \
	done

# install: documentation
$(DESTDIR)/doc/index.html: $(LIBRARIES) doc/index.html
	make doc
	mkdir -p $(DESTDIR)/doc
	cp -rf doc/* $(DESTDIR)/doc
	cp -rf TODO.txt $(DESTDIR)/doc

$(LIBRARIES): $(CFGLIB_OBJ)
	ar rcs $@ $^


cleandoc:
	$(RM) -R doc/generated-doc/html

