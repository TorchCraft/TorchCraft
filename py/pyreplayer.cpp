#include "pytorchcraft.h"

#include "frame.h"
#include "replayer.h"

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
      .def_readwrite("total_psi", &Resources::total_psi);

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
      .def_readwrite("reward", &Frame::reward)
      .def_readwrite("is_terminal", &Frame::is_terminal)
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
      .def(
          "save",
          &Replayer::save,
          py::arg("path"),
          py::arg("compressed") = true);

  m_sub.def("load", [](const std::string& path) {
    auto rep = new Replayer();
    rep->load(path);
    return rep;
  });
}
