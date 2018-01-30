#include "pytorchcraft.h"

#include "client.h"
#include "state.h"

using namespace torchcraft;

void init_client(py::module& torchcraft) {
  py::class_<Client> client(torchcraft, "Client");

  client.def(py::init<>())
      .def(
          "connect",
          &Client::connect,
          py::arg("hostname"),
          py::arg("port") = 11111,
          py::arg("timeout") = -1)
      .def("connected", &Client::connected)
      .def("close", &Client::close)
      .def(
          "init",
          [](Client* self,
             std::pair<int, int> window_size,
             std::pair<int, int> window_pos,
             bool micro_battles) {
            Client::Options opts;
            opts.window_size[0] = window_size.first;
            opts.window_size[1] = window_size.second;
            opts.window_pos[0] = window_pos.first;
            opts.window_pos[1] = window_pos.second;
            opts.micro_battles = micro_battles;

            std::vector<std::string> updates;
            if (self->init(updates, opts)) {
              return py::cast(self->state());
            }
            return py::cast(nullptr);
          },
          py::arg("window_size") = std::make_tuple(-1, -1),
          py::arg("window_pos") = std::make_tuple(-1, -1),
          py::arg("micro_battles") = false,
          py::return_value_policy::reference_internal)
      .def(
          "send",
          [](Client* self, std::vector<std::vector<py::object>> commands) {
            std::vector<Client::Command> to_send;
            for (auto vec : commands) {
              if (vec.size() == 0) {
                continue;
              }

              Client::Command cmd;
              cmd.code = vec[0].cast<int>();
              if (vec.size() > 1) {
                try {
                  auto arg1 = vec[1].cast<std::string>();
                  cmd.str = arg1;
                } catch (pybind11::cast_error& e) {
                  cmd.args.push_back(vec[1].cast<int>());
                }
                for (size_t i = 2; i < vec.size(); i++) {
                  cmd.args.push_back(vec[i].cast<int>());
                }
              }
              to_send.push_back(cmd);
            }
            return self->send(to_send);
          })
      .def(
          "recv",
          [](Client* self) {
            std::vector<std::string> updates;
            if (!self->receive(updates)) {
              throw std::runtime_error(
                  std::string("Receive failure: ") + self->error());
            }
            return self->state();
          },
          py::return_value_policy::reference_internal)
      .def("poll", &Client::poll)
      .def("error", &Client::error)
      .def(
          "state", &Client::state, py::return_value_policy::reference_internal);
}
