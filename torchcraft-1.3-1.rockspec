package = "TorchCraft"
version = "1.3-1"
source = {
    url = "git://github.com/torchcraft/torchcraft",
    tag = "v1.3-1",
}
description = {
    summary = "Connects Torch to StarCraft through BWAPI",
    detailed = [[
        Connects Torch to StarCraft through BWAPI,
        allows to receive StarCraft state and send it command.
    ]],
    homepage = "http://github.com/TorchCraft/TorchCraft",
    license = "BSD 3-clause"
}
dependencies = {
    "penlight",
    "torch >= 7.0",
}
build = {
    type = "command",
    build_command = [[
    cmake -E make_directory build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DLUALIB=$(LUALIB) -DLUA_INCDIR="$(LUA_INCDIR)" -DLUA_LIBDIR="$(LUA_LIBDIR)" -DLUADIR="$(LUADIR)" -DLIBDIR="$(LIBDIR)" -DCMAKE_INSTALL_PREFIX="$(PREFIX)" && $(MAKE)
    ]],
    install_command = "cd build && $(MAKE) install",
    install = {
      lua = { 
        ["torchcraft._env"] = "lua/_env.lua",
        ["torchcraft.init"] = "lua/init.lua",
        ["torchcraft.replayer"] = "lua/replayer.lua",
        ["torchcraft.utils"] = "lua/utils.lua"
      }
    }
}

