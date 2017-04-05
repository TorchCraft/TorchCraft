#include "pytorchcraft.h"

#include "replayer.h"
#include "frame.h"

#include <pybind11/operators.h>
using namespace torchcraft::replayer;

void init_replayer(py::module &m) {
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
    .def("getFlag", [](const Unit &u, Unit::Flags flag) {
        return (u.flags & flag) > 0;
    })
    .def("setFlag", [](Unit &u, Unit::Flags flag, bool value) {
        if (value) u.flags |= flag;
        else u.flags &= (~flag);
    });

  py::enum_<Unit::Flags>(unit, "Flags")
    .value("Accelerating", Unit::Flags::Accelerating)
    .value("Attacking", Unit::Flags::Attacking)
    .value("AttackFrame", Unit::Flags::AttackFrame)
    .value("BeingConstructed", Unit::Flags::BeingConstructed)
    .value("BeingGathered", Unit::Flags::BeingGathered)
    .value("BeingHealed", Unit::Flags::BeingHealed)
    .value("Blind", Unit::Flags::Blind)
    .value("Braking", Unit::Flags::Braking)
    .value("Burrowed", Unit::Flags::Burrowed)
    .value("CarryingGas", Unit::Flags::CarryingGas)
    .value("CarryingMinerals", Unit::Flags::CarryingMinerals)
    .value("Cloaked", Unit::Flags::Cloaked)
    .value("Completed", Unit::Flags::Completed)
    .value("Constructing", Unit::Flags::Constructing)
    .value("DefenseMatrixed", Unit::Flags::DefenseMatrixed)
    .value("Detected", Unit::Flags::Detected)
    .value("Ensnared", Unit::Flags::Ensnared)
    .value("Flying", Unit::Flags::Flying)
    .value("Following", Unit::Flags::Following)
    .value("GatheringGas", Unit::Flags::GatheringGas)
    .value("GatheringMinerals", Unit::Flags::GatheringMinerals)
    .value("Hallucination", Unit::Flags::Hallucination)
    .value("HoldingPosition", Unit::Flags::HoldingPosition)
    .value("Idle", Unit::Flags::Idle)
    .value("Interruptible", Unit::Flags::Interruptible)
    .value("Invincible", Unit::Flags::Invincible)
    .value("Irradiated", Unit::Flags::Irradiated)
    .value("Lifted", Unit::Flags::Lifted)
    .value("Loaded", Unit::Flags::Loaded)
    .value("LockedDown", Unit::Flags::LockedDown)
    .value("Maelstrommed", Unit::Flags::Maelstrommed)
    .value("Morphing", Unit::Flags::Morphing)
    .value("Moving", Unit::Flags::Moving)
    .value("Parasited", Unit::Flags::Parasited)
    .value("Patrolling", Unit::Flags::Patrolling)
    .value("Plagued", Unit::Flags::Plagued)
    .value("Powered", Unit::Flags::Powered)
    .value("Repairing", Unit::Flags::Repairing)
    .value("Researching", Unit::Flags::Researching)
    .value("Selected", Unit::Flags::Selected)
    .value("Sieged", Unit::Flags::Sieged)
    .value("StartingAttack", Unit::Flags::StartingAttack)
    .value("Stasised", Unit::Flags::Stasised)
    .value("Stimmed", Unit::Flags::Stimmed)
    .value("Stuck", Unit::Flags::Stuck)
    .value("Targetable", Unit::Flags::Targetable)
    .value("Training", Unit::Flags::Training)
    .value("UnderAttack", Unit::Flags::UnderAttack)
    .value("UnderDarkSwarm", Unit::Flags::UnderDarkSwarm)
    .value("UnderDisruptionWeb", Unit::Flags::UnderDisruptionWeb)
    .value("UnderStorm", Unit::Flags::UnderStorm)
    .value("Upgrading", Unit::Flags::Upgrading)
    .export_values();

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
    .def("getFrame", &Replayer::getFrame)
    .def("push", &Replayer::push)
    .def("setKeyFrame", &Replayer::setKeyFrame)
    .def("getKeyFrame", &Replayer::getKeyFrame)
    .def("setNumUnits", &Replayer::setNumUnits)
    .def("getNumUnits", &Replayer::getNumUnits)
    .def("save", &Replayer::save,
        py::arg("path"),
        py::arg("compressed") = true
        );

  m_sub.def("load", [](const std::string& path) {
      auto rep = new Replayer();
      rep->load(path);
      return rep;
  });
}
