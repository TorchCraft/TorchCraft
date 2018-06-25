This is a simple doc of the communication protocol between the Torch client and the StarCraft server.

## Messages order

The order in which one starts StarCraft and Torch should not matter. The communication between both sides is done through ZMQ. The ZMQ server is owned by the StarCraft side (AIClient or AIModule). The Torch side should always call `connect()` and then alternate between `send` and `receive` (we use the [request-reply pattern](http://zguide.zeromq.org/page:all#Ask-and-Ye-Shall-Receive)). Here is an example in action.

    tc = require 'torchcraft'
    tc:init(hostname, port) -- not compulsory
    tc:connect(port)
    settings = {tc.command(tc.set_speed, 0),
                tc.set_combine_frames, 7}
    tc:send(settings)
    while true do
        tc:receive() -- will update tc.state
        commands = compute_commands(tc.state)
        tc:send(tc.command(commands))
    end
        
What you get from the game: `tc.state` (the full raw content of what you get is in `tc.state.frame`).
What you can send: `tc.unitcommandtypes` and `tc.usercommandtype`.

<!-- TODO detail all the functions / state / commands of TorchCraft -->
