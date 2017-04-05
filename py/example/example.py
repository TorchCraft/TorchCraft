import torchcraft
u = torchcraft.replayer.Unit()
u.setFlag(u.Flags.Accelerating, True)
print(u.getFlag(u.Flags.Accelerating))

rep = torchcraft.replayer.load("bwrep_gwhxe.rep.tcr")
