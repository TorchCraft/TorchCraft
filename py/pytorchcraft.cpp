#include "pytorchcraft.h"

#include "client.h"

void init_replayer(py::module &);
void init_constants(py::module &);
void init_state(py::module &);
void init_client(py::module &);

PYBIND11_PLUGIN(torchcraft) {
  torchcraft::init();
  py::module m("torchcraft", "Interfaces torchcraft to python");

  init_replayer(m);
  init_constants(m);
  init_state(m);
  init_client(m);

  return m.ptr();
}
