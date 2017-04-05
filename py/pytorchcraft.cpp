#include "pytorchcraft.h"

void init_replayer(py::module &);
void init_constants(py::module &);

PYBIND11_PLUGIN(torchcraft) {
  py::module m("torchcraft", "Interfaces torchcraft to python");

  init_replayer(m);
  init_constants(m);

  return m.ptr();
}
