#!/bin/sh
flatc -c --gen-mutable --scoped-enums --gen-object-api --no-includes messages.fbs
sed -i "" 's,\#include "flatbuffers/flatbuffers.h",\#include "flatbuffers.h",' messages_generated.h
