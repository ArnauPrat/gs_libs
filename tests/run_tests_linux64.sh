#/!bin/bash

set -e

# This script is bound to VIM F9 key to execute the compilation into the
# build directory

TARGET=""

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

echo "RUNNING TESTS WITH TARGET ${TARGET}

BUILD_DIR="build_linux64_${TARGET}"
TESTS="gs_mem_alloc_test"

for a in ${TESTS} 
do
  echo "Executing ${a} test"
  ${BUILD_DIR}/${a} 
done
exit 0
