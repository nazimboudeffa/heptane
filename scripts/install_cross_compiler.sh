#!/bin/sh

# install_cross_compiler.sh [ LINUX | MACOS] [ MIPS | ARM ] ${CROSS_COMPILERS_DIR}
ARCH=$2
OS=$1
CC_DIR=${ARCH}_${OS}
CROSS_COMPILERS_DIR=$3

if [ -e "${CROSS_COMPILERS_DIR}/${CC_DIR}" ]; then
    echo " The cross compiler is already installed ( directory : ${CROSS_COMPILERS_DIR}/${CC_DIR} )"
    bcont="YES"
    while [ "$bcont" = "YES" ] ; do
      echo " Do you want to replace it [y/n] ?"
      read rep
      if [ "$rep" = "y" ]; then bcont="NO"; elif [ "$rep" = "n" ]; then bcont="NO" ; fi
    done
     if [ "$rep" = "y" ]; then
	 echo "Deleting ${CROSS_COMPILERS_DIR}/${CC_DIR} "
	 rm -rf ${CROSS_COMPILERS_DIR}/${CC_DIR}
     fi
else
    rep="y"
    mkdir -p ${CROSS_COMPILERS_DIR}
fi

if [ "$rep" = "y" ]; then
    cd ${CROSS_COMPILERS_DIR}
    FILENAME=CROSS_COMPILER_${CC_DIR}.tar.bz2
    echo " *** Cross Compiler $ARCH target for $OS: Downloading "    
    wget -c http://www.irisa.fr/alf/downloads/heptane/CROSS_COMPILERS/${FILENAME}
    
    
    if [ -e "${FILENAME}" ]; then
	echo " ***  Cross Compiler: Installing"
	tar xfj ${FILENAME}
	rm -f ${FILENAME}
	rm -f ${ARCH}
	ln -s ${CC_DIR} ${ARCH}
    fi
fi
