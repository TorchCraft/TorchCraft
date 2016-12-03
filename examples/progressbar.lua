--[[
   Copyright (c) 2015-present, Facebook, Inc.
   All rights reserved.
   This source code is licensed under the BSD-style license found in the
   LICENSE file in the root directory of this source tree. An additional grant
   of patent rights can be found in the PATENTS file in the same directory.
]]--

local tablex = require 'pl.tablex'
local torch = require 'torch'
local progress = torch.class('ProgressBar')
local tds=require 'tds'
local Threads = require 'threads'

function progress:__init(id)
    self.stringstore = tds.hash()
    self.mutex_id = id
end

function progress:add(key, value, format)
    self.data = self.data or {}
    self.format = self.format or {}
    self.keys = self.keys or {}
    if not tablex.find(self.keys, key) then
        self.keys[#self.keys+1] = key
    end
    self.data[key] = value
    self.format[key] = format
end

function progress:push()
    self.keys = self.keys or {}
    local str = ''

    for i=1,#self.keys do
        local key = self.keys[i]
        local format = self.format[key] or '%s'
        local value = self.data[key]
        local s = string.format('%s = ' .. format, key, value)
        str = str .. s .. ' | '
    end

    self.keys = {}
    self.data = {}
    self.format = {}
    self.mutex = self.mutex or Threads.Mutex(self.mutex_id)
    self.mutex:lock()
    self.stringstore['str'] = str
    self.mutex:unlock()
    if self.singlethread then self:pop() end
end

function progress:pop()
    self.mutex = self.mutex or Threads.Mutex(self.mutex_id)
    self.mutex:lock()
    local str = self.stringstore['str']
    self.mutex:unlock()
    if str == nil then
        return
    end
    if not self.started then
        self.started = true;
        io.write('\n')
    end
    if self.length then
        for i=1,self.length do
            io.write('\b')
        end
    end
    io.write(str)
    self.length  = #str
end

function progress:singleThreaded()
    self.singlethread = true
end

function progress:reset()
    self:push()
end

return progress
