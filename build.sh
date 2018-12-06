#!/usr/bin/env bash

BUILD_DIR="${PWD}/build"

if [ ! -d "${BUILD_DIR}" ]; then
  if ! mkdir -p "${BUILD_DIR}"
  then
     printf "Unable to create build directory %s.\\n Exiting now.\\n" "${BUILD_DIR}"
     exit 1;
  fi
fi

cd ${BUILD_DIR}
cmake ..
make
cd ..
