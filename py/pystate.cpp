#include "pytorchcraft.h"

#include "state.h"

using namespace torchcraft;

void init_state(py::module& torchcraft) {
  py::class_<State> state(torchcraft, "State");
  py::class_<State::Position>(state, "Position")
      .def(py::init<>())
      .def(py::init<int, int>())
      .def_readwrite("x", &State::Position::x)
      .def_readwrite("y", &State::Position::y);

  state.def_readwrite("lag_frames", &State::lag_frames)
      .def_property_readonly(
          "map_size",
          [](State* self) {
            return py::make_tuple(self->map_size[0], self->map_size[1]);
          })
      // TODO Maybe expose these _data arrays as THTensors.
      .def_readwrite("ground_height_data", &State::ground_height_data)
      .def_readwrite("walkable_data", &State::walkable_data)
      .def_readwrite("buildable_data", &State::buildable_data)
      .def_readwrite("map_name", &State::map_name)
      .def_readwrite("start_locations", &State::start_locations)
      .def_readwrite("player_races", &State::player_races)
      .def_readwrite("player_names", &State::player_names)
      .def_readwrite("player_id", &State::player_id)
      .def_readwrite("neutral_id", &State::neutral_id)
      .def_readwrite("replay", &State::replay)
      .def_readwrite("frame_string", &State::frame_string)
      .def_readwrite("deaths", &State::deaths)
      .def_readwrite("frame_from_bwapi", &State::frame_from_bwapi)
      .def_readwrite("battle_frame_count", &State::battle_frame_count)
      .def_readwrite("game_ended", &State::game_ended)
      .def_readwrite("game_won", &State::game_won)
      .def_readwrite("battle_just_ended", &State::battle_just_ended)
      .def_readwrite("battle_won", &State::battle_won)
      .def_readwrite("waiting_for_restart", &State::waiting_for_restart)
      .def_readwrite("last_battle_ended", &State::last_battle_ended)
      .def_readwrite("img_mode", &State::img_mode)
      .def_property("screen_position", RWPAIR(State, screen_position, int))
      .def_readwrite("visibility", &State::visibility)
      .def_property("visibility_size", RWPAIR(State, visibility_size, int))
      .def_readwrite("image", &State::image)
      .def_property("image_size", RWPAIR(State, image_size, int))
      .def_readwrite("aliveUnits", &State::aliveUnits)
      .def_readwrite("aliveUnitsConsidered", &State::aliveUnitsConsidered)
      .def_readwrite("units", &State::units)
      .def_readonly("frame", &State::frame)
      .def(
          py::init<bool, std::set<BW::UnitType>>(),
          py::arg("microBattles") = false,
          py::arg("onlyConsideredTypes") = std::set<BW::UnitType>())
      .def_property(
          "only_consider_types",
          &State::onlyConsiderTypes,
          &State::setOnlyConsiderTypes)
      .def("reset", &State::reset)
      .def("clone", [](State* self) { return new State(*self); });
}
