#include "pytorchcraft.h"

void init_replayer(py::module &);

PYBIND11_PLUGIN(torchcraft) {
  py::module m("torchcraft", "Interfaces torchcraft to python");

  init_replayer(m);

  return m.ptr();
}
