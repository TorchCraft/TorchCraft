#include "pytorchcraft.h"

#include "frame.h"
#include "replayer.h"

#include <pybind11/numpy.h>
#include <pybind11/operators.h>
using namespace torchcraft::replayer;

void init_replayer(py::module& m) {
  py::module m_sub = m.def_submodule("replayer");
  py::class_<Order>(m_sub, "Order")
      .def(py::init<>())
      .def_readwrite("first_frame", &Order::first_frame)
      .def_readwrite("type", &Order::type)
      .def_readwrite("targetId", &Order::targetId)
      .def_readwrite("targetX", &Order::targetX)
      .def_readwrite("targetY", &Order::targetY)
      .def(py::self == py::self);

  py::class_<UnitCommand>(m_sub, "UnitCommand")
      .def(py::init<>())
      .def_readwrite("frame", &UnitCommand::frame)
      .def_readwrite("type", &UnitCommand::type)
      .def_readwrite("targetId", &UnitCommand::targetId)
      .def_readwrite("targetX", &UnitCommand::targetX)
      .def_readwrite("targetY", &UnitCommand::targetY)
      .def_readwrite("extra", &UnitCommand::extra)
      .def(py::self == py::self);

  py::class_<Unit> unit(m_sub, "Unit");
  unit.def(py::init<>())
      .def_readwrite("id", &Unit::id)
      .def_readwrite("x", &Unit::x)
      .def_readwrite("y", &Unit::y)
      .def_readwrite("flags", &Unit::flags)
      .def_readwrite("health", &Unit::health)
      .def_readwrite("max_health", &Unit::max_health)
      .def_readwrite("shield", &Unit::shield)
      .def_readwrite("max_shield", &Unit::max_shield)
      .def_readwrite("energy", &Unit::energy)
      .def_readwrite("maxCD", &Unit::maxCD)
      .def_readwrite("groundCD", &Unit::groundCD)
      .def_readwrite("airCD", &Unit::airCD)
      .def_readwrite("visible", &Unit::visible)
      .def_readwrite("type", &Unit::type)
      .def_readwrite("armor", &Unit::armor)
      .def_readwrite("shieldArmor", &Unit::shieldArmor)
      .def_readwrite("size", &Unit::size)
      .def_readwrite("pixel_x", &Unit::pixel_x)
      .def_readwrite("pixel_y", &Unit::pixel_y)
      .def_readwrite("pixel_size_x", &Unit::pixel_size_x)
      .def_readwrite("pixel_size_y", &Unit::pixel_size_y)
      .def_readwrite("groundATK", &Unit::groundATK)
      .def_readwrite("airATK", &Unit::airATK)
      .def_readwrite("groundDmgType", &Unit::groundDmgType)
      .def_readwrite("airDmgType", &Unit::airDmgType)
      .def_readwrite("groundRange", &Unit::groundRange)
      .def_readwrite("airRange", &Unit::airRange)
      .def_readwrite("orders", &Unit::orders)
      .def_readwrite("command", &Unit::command)
      .def_readwrite("velocityX", &Unit::velocityX)
      .def_readwrite("velocityY", &Unit::velocityY)
      .def_readwrite("playerId", &Unit::playerId)
      .def_readwrite("resources", &Unit::resources)
      .def(
          "getFlag",
          [](const Unit& u, Unit::Flags flag) { return (u.flags & flag) > 0; })
      .def("setFlag", [](Unit& u, Unit::Flags flag, bool value) {
        if (value)
          u.flags |= flag;
        else
          u.flags &= (~flag);
      });

  std::string tmp;
#define DO_FLAG(FLAG)                                                   \
  tmp = fromCamelCaseToLower(#FLAG);                                    \
  unit.def_property(                                                    \
      tmp.c_str(),                                                      \
      [](Unit* self) { return (self->flags & Unit::Flags::FLAG) > 0; }, \
      [](Unit* self, bool value) {                                      \
        if (value)                                                      \
          self->flags |= Unit::Flags::FLAG;                             \
        else                                                            \
          self->flags &= (~Unit::Flags::FLAG);                          \
      })

  DO_FLAG(Accelerating);
  DO_FLAG(Attacking);
  DO_FLAG(AttackFrame);
  DO_FLAG(BeingConstructed);
  DO_FLAG(BeingGathered);
  DO_FLAG(BeingHealed);
  DO_FLAG(Blind);
  DO_FLAG(Braking);
  DO_FLAG(Burrowed);
  DO_FLAG(CarryingGas);
  DO_FLAG(CarryingMinerals);
  DO_FLAG(Cloaked);
  DO_FLAG(Completed);
  DO_FLAG(Constructing);
  DO_FLAG(DefenseMatrixed);
  DO_FLAG(Detected);
  DO_FLAG(Ensnared);
  DO_FLAG(Flying);
  DO_FLAG(Following);
  DO_FLAG(GatheringGas);
  DO_FLAG(GatheringMinerals);
  DO_FLAG(Hallucination);
  DO_FLAG(HoldingPosition);
  DO_FLAG(Idle);
  DO_FLAG(Interruptible);
  DO_FLAG(Invincible);
  DO_FLAG(Irradiated);
  DO_FLAG(Lifted);
  DO_FLAG(Loaded);
  DO_FLAG(LockedDown);
  DO_FLAG(Maelstrommed);
  DO_FLAG(Morphing);
  DO_FLAG(Moving);
  DO_FLAG(Parasited);
  DO_FLAG(Patrolling);
  DO_FLAG(Plagued);
  DO_FLAG(Powered);
  DO_FLAG(Repairing);
  DO_FLAG(Researching);
  DO_FLAG(Selected);
  DO_FLAG(Sieged);
  DO_FLAG(StartingAttack);
  DO_FLAG(Stasised);
  DO_FLAG(Stimmed);
  DO_FLAG(Stuck);
  DO_FLAG(Targetable);
  DO_FLAG(Training);
  DO_FLAG(UnderAttack);
  DO_FLAG(UnderDarkSwarm);
  DO_FLAG(UnderDisruptionWeb);
  DO_FLAG(UnderStorm);
  DO_FLAG(Upgrading);
#undef DO_FLAG

  py::class_<Resources>(m_sub, "Resources")
      .def(py::init<>())
      .def_readwrite("ore", &Resources::ore)
      .def_readwrite("gas", &Resources::gas)
      .def_readwrite("used_psi", &Resources::used_psi)
      .def_readwrite("total_psi", &Resources::total_psi)
      .def_readwrite("upgrades", &Resources::upgrades)
      .def_readwrite("upgrades_level", &Resources::upgrades_level)
      .def_readwrite("techs", &Resources::techs);

  py::class_<Bullet>(m_sub, "Bullet")
      .def(py::init<>())
      .def_readwrite("type", &Bullet::type)
      .def_readwrite("x", &Bullet::x)
      .def_readwrite("y", &Bullet::y);

  py::class_<Action>(m_sub, "Action")
      .def(py::init<>())
      .def_readwrite("action", &Action::action)
      .def_readwrite("uid", &Action::uid)
      .def_readwrite("aid", &Action::aid);

  py::class_<Frame>(m_sub, "Frame")
      .def(py::init<>())
      .def(py::init<Frame*>())
      .def_readwrite("units", &Frame::units)
      .def_readwrite("actions", &Frame::actions)
      .def_readwrite("resources", &Frame::resources)
      .def_readwrite("bullets", &Frame::bullets)
      .def_readwrite("height", &Frame::bullets)
      .def_readwrite("width", &Frame::bullets)
      .def_readwrite("reward", &Frame::reward)
      .def_readwrite("is_terminal", &Frame::is_terminal)
      .def("get_creep_at", &Frame::getCreepAt)
      .def("combine", &Frame::combine)
      .def("filter", &Frame::filter);

  py::class_<Replayer>(m_sub, "Replayer")
      .def(py::init<>())
      .def("__len__", &Replayer::size)
      .def(
          "getFrame",
          &Replayer::getFrame,
          py::return_value_policy::reference_internal)
      .def("push", &Replayer::push)
      .def("setKeyFrame", &Replayer::setKeyFrame)
      .def("getKeyFrame", &Replayer::getKeyFrame)
      .def("setNumUnits", &Replayer::setNumUnits)
      .def("getNumUnits", &Replayer::getNumUnits)
      .def("setMapFromState", &Replayer::setMapFromState)
      .def(
          "setMap",
          [](Replayer* self, py::dict inp) {
            py::object wobj = inp["walkability"];
            py::object bobj = inp["buildability"];
            py::object gobj = inp["ground_height"];
            auto walkability = static_cast<py::array_t<uint8_t>*>(&wobj);
            auto buildability = static_cast<py::array_t<uint8_t>*>(&bobj);
            auto ground_height = static_cast<py::array_t<uint8_t>*>(&gobj);
            auto w_data = walkability->unchecked<2>();
            auto g_data = ground_height->unchecked<2>();
            auto b_data = buildability->unchecked<2>();
            std::vector<uint8_t> winp, ginp, binp;

            uint64_t h = w_data.shape(0);
            uint64_t w = w_data.shape(1);

            for (size_t y = 0; y < h; y++) {
              for (size_t x = 0; x < w; x++) {
                winp.push_back(w_data(y, x));
                ginp.push_back(g_data(y, x));
                binp.push_back(b_data(y, x));
              }
            }

            auto start_loc =
                inp["start_locations"].cast<std::vector<std::pair<int, int>>>();
            std::vector<int> slx, sly;
            for (auto p : start_loc) {
              slx.push_back(p.first);
              sly.push_back(p.second);
            }

            self->setMap(h, w, winp.data(), ginp.data(), binp.data(), slx, sly);
          })
      .def(
          "getMap",
          [](Replayer* self) {
#define WALKABILITY_SHIFT 0
#define BUILDABILITY_SHIFT 1
#define HEIGHT_SHIFT 2
// height is 0-5, hence 3 bits
#define START_LOC_SHIFT 5

            // TODO Figure out how to return a THTensor... Copying the code
            // avoids an extra copy operation
            const auto map = self->getRawMap();
            auto h = (uint64_t)THByteTensor_size(map, 0);
            auto w = (uint64_t)THByteTensor_size(map, 1);
            auto walkability = py::array_t<uint8_t>({h, w});
            auto ground_height = py::array_t<uint8_t>({h, w});
            auto buildability = py::array_t<uint8_t>({h, w});
            auto w_data = walkability.mutable_unchecked<2>();
            auto g_data = ground_height.mutable_unchecked<2>();
            auto b_data = buildability.mutable_unchecked<2>();

            std::vector<std::pair<int, int>> start_loc;
            for (size_t y = 0; y < h; y++) {
              for (size_t x = 0; x < w; x++) {
                uint8_t v = THTensor_fastGet2d(map, y, x);
                w_data(y, x) = (v >> WALKABILITY_SHIFT) & 1;
                b_data(y, x) = (v >> BUILDABILITY_SHIFT) & 1;
                g_data(y, x) = (v >> HEIGHT_SHIFT) & 0b111;
                bool is_start = ((v >> START_LOC_SHIFT) & 1) == 1;
                if (is_start)
                  start_loc.emplace_back(x, y);
              }
            }

            py::dict ret;
            ret[py::str("walkability")] = walkability;
            ret[py::str("buildability")] = buildability;
            ret[py::str("ground_height")] = ground_height;
            ret[py::str("start_locations")] = start_loc;
            return ret;
          })
      .def(
          "save",
          &Replayer::save,
          py::arg("path"),
          py::arg("compressed") = true);

  m_sub.def("load", [](const std::string& path) {
    py::gil_scoped_release release;
    auto rep = new Replayer();
    rep->load(path);
    return rep;
  });
}
