#include "pytorchcraft.h"

#include "client.h"
#include "state.h"

using namespace torchcraft;

void init_client(py::module &torchcraft) {
  py::class_<Client> client(torchcraft, "Client");

  client.def(py::init<>())
    .def("connect", &Client::connect, py::arg("hostname"), py::arg("port") = 11111, py::arg("timeout") = -1)
    .def("connected", &Client::connected)
    .def("close", &Client::close)
    .def("init",
        [](Client *self, std::pair<int, int> window_size,
            std::pair<int, int> window_pos, bool micro_battles) {
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
        py::arg("micro_battles") = false
        )
    .def("send",
        [](Client *self, std::vector<std::vector<py::object>> commands) {
          std::vector<Client::Command> to_send;
          for (auto vec : commands) {
            auto arg0 = vec[0].cast<int>();
            if (vec.size() == 0) continue;
            else if (vec.size() == 1) to_send.emplace_back(arg0);
            else {
              std::vector<int> the_rest;
              for (size_t i=2; i < vec.size(); i++) the_rest.push_back(vec[i].cast<int>());
              try {
                auto arg1 = vec[1].cast<std::string>();
                to_send.emplace_back(arg0, arg1, the_rest);
              } catch (pybind11::cast_error& e) {
                the_rest.insert(the_rest.begin(), vec[1].cast<int>());
                to_send.emplace_back(arg0, "", the_rest);
              }
            }
          }
          return self->send(to_send);
        })
    .def("recv", [](Client *self) {
        std::vector<std::string> updates;
        if (!self->receive(updates)) {
          throw std::runtime_error(std::string("Receive failure: ") + self->error());
        }
        return self->state();
        })
    .def("poll", &Client::poll)
    .def("error", &Client::error)
    .def("state", &Client::state)
    ;
}

