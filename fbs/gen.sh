#!/bin/sh
flatc -c --gen-mutable --scoped-enums --gen-object-api --no-includes bwenv_messages.fbs
sed -i 's,\#include "flatbuffers/flatbuffers.h",\#include "flatbuffers.h",' bwenv_messages_generated.h
