# Torchcraft

This the alpha version of pytorchcraft v0.0.1, and current is parity with the
develop branch of torchcraft

## What's here:
- Direct C++ bindings to client/*.cpp, plus a few python extras
- init and recv recevies torchcraft States
- Most of the work is a very thin wrapper with no overhead. Be careful that
  pointers will still be in pointers in C++. Specifically `state.frame` will
  remain the same pointer and be overwritten at every recv.
- Everything _should_ work.

## What's not here
- Tests...
- A robust setup.py


# Requirements

C++ Requirements:
  - zstd-devel 1.1.4
  - zeromq 4+

Python requirements:
  - pybind11
