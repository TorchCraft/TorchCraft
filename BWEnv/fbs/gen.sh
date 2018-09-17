#!/bin/bash
flatcVersionRequired='1.8.0';
flatcVersionCurrent=`flatc --version`;
if [[ $flatcVersionCurrent == *"$flatcVersionRequired"* ]]; then
  flatc -c --gen-mutable --scoped-enums --gen-object-api --no-includes *.fbs
  sed -i 's,\#include "flatbuffers/flatbuffers.h",\#include "flatbuffers.h",' *.h
else
  echo "The current flatbuffer headers require flatc version $flatcVersionRequired."
  echo "Current flatbuffer version: $flatcVersionCurrent"
  echo "You'll either want to get a compatible copy of flatc or update the headers."
fi
