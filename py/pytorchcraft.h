#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#define READPAIR(TYP, VAR) \
  [](TYP* self) { return py::make_tuple(self->VAR[0], self->VAR[1]); }

#define WRITEPAIR(TYP, VAR, INNER)              \
  [](TYP* self, std::pair<INNER, INNER> VAR) {  \
  self->VAR[0] = VAR.first;                     \
  self->VAR[1] = VAR.second;                    \
}

#define RWPAIR(TYP, VAR, INNER) READPAIR(TYP, VAR), WRITEPAIR(TYP, VAR, INNER)

namespace py = pybind11;
