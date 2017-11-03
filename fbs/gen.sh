#!/bin/sh
flatc -c --gen-mutable --scoped-enums --gen-object-api --no-includes torchcraft.fbs
