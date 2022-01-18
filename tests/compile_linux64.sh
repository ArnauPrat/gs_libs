#!/bin/bash 
set -e

#This script is bound to VIM F9 key to execute the compilation into the
# build directory

EXITCODE=0
TARGET=""
CLANG_OPTIONS=""
INCLUDES="-I ../"

#"Processing script parameters"
while [[ $# > 0 ]]
do
	key="$1"
	case $key in
		-t)
			TARGET="$2"
			shift # past argument
			;;
	esac
	shift
done

if [ -z ${TARGET} ] 
then
    echo "Target not defined"
    exit 1
fi

if [ ${TARGET} == "DEBUG" ]
then
  CLANG_OPTIONS="-O0 -g -pg"
fi

if [ ${TARGET} == "DEBUG" ]
then
  CLANG_OPTIONS="-O2 -DNDEBUG"
fi

echo "BUILING TESTS WITH TARGET %TARGET%"

BUILD_DIR="build_linux64_${TARGET}"
mkdir -p ${BUILD_DIR}


TESTS="gs_mem_alloc_test"

for a in ${TESTS} 
do
  echo "clang ${INCLUDES} ${CLANG_OPTIONS} -o ${BUILD_DIR}/$a ${a}.c"
  clang ${INCLUDES} ${CLANG_OPTIONS} -o ${BUILD_DIR}/${a} ${a}.c
done
exit 0
