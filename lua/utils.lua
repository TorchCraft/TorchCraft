local pl = require 'pl.import_into'()

local utils = {}
-- check if table is empty
function utils.isEmpty(t)
    assert(type(t) == 'table',
           "type of input has to be table, but got " .. type(t))
    return next(t) == nil
end

-- check if all of table's existing elements are e
-- if any element is ~= e then returns false.
function utils.last_elem(t, e)
    assert(t)
    assert(e)
    for k, _ in pairs(t) do
        if k ~= e then
            return false
        end
    end
    return true
end

-- returns a table indexed from 1 to the number of keys in tab
-- tabsize must be the number of elements in tab, or nil
function utils.shuffle_keys(tab, tabsize)
    if tabsize == nil then
        tabsize = 0
        for _, _ in pairs(tab) do tabsize = tabsize + 1 end
    end
    local indexes = torch.randperm(tabsize)
    local my_perm = {}
    local i=0
    for k, _ in pairs(tab) do
        i = i + 1
        my_perm[indexes[i]] = k
    end
    return my_perm
end

-- get the center of the units
function utils.get_units_center(units, nested)
    local x, y = 0, 0
    local n_units = 0
    local function iterate_over(this_units)
        for _, feats in pairs(this_units) do
            x = x + feats.position[1]
            y = y + feats.position[2]
            n_units = n_units + 1
        end
    end
    if not nested then
        iterate_over(units)
    else
        for _, this_units in pairs(units) do
            iterate_over(this_units)
        end
    end
    assert(n_units > 0)
    return x / n_units, y / n_units
end

-- get weakest of a bunch of units
function utils.get_weakest(unitsTable)
    local min_total_HP = 1E30
    local weakest_uid = nil
    for uid, ut in pairs(unitsTable) do
        local tmp_hp = ut['hp'] + ut['shield']
        if tmp_hp < min_total_HP then
            min_total_HP = tmp_hp
            weakest_uid = uid
        end
    end
    return weakest_uid
end

-- distance given two tables {x,y}
function utils.distance(pos1, pos2)
    local x1, y1 = unpack(pos1)
    local x2, y2 = unpack(pos2)
    local dx = x1 - x2
    local dy = y1 - y2
    return math.sqrt(dx * dx + dy * dy)
end

-- get the closest units from `unitsTable` to `position`
function utils.get_closest(position, unitsTable)
    local min_d = 1E30
    local closest_uid = nil
    for uid, ut in pairs(unitsTable) do
        local tmp_d = utils.distance(position, ut['position'])
        if tmp_d < min_d then
            min_d = tmp_d
            closest_uid = uid
        end
    end
    return closest_uid
end

-- in-place cudification of tensors
function utils.cudify_table(table_of_tensors)
    for k, v in pairs(table_of_tensors) do
        table_of_tensors[k] = table_of_tensors[k]:cuda()
    end
    return table_of_tensors
end

-- recursive cudification
function utils.deep_cudify(tab)
    if tab.cuda ~= nil then
        return tab:cuda()
    elseif type(tab) == 'table' then
        for k, v in pairs(tab) do
            tab[k] = utils.deep_cudify(v)
        end
    else
        error('Unrecognized argument type in Featurizer.cudify: '
                  .. type(tab))
    end
    return tab
end

-- debug/test utilities
if g_DEBUG_CMD then
    function utils.assertfalse_debug_opt(opt_name, debug_cmd)
        assert(debug_cmd ~= nil, 'debug inconsistencies '
                   .. '-- check your debug commands/variables')
        if debug_cmd:find(pl.utils.escape(opt_name)) then
            assert(false, 'invalid code path with option ' .. opt_name)
        end
    end
    function utils.assert_debug_condition(to_print, condition)
        assert(g_DEBUG_CMD ~= nil and g_DEBUG_CMD ~= '',
               'bad usage of debug')
        if not condition() then
            print('test condition does not match')
            print(to_print)
            error('error while testing; see previous lines for details')
        end
    end
else
    function utils.assertfalse_debug_opt(x)
        -- identity
        return x
    end
    function utils.assert_debug_condition(condition)
       --does nothing
    end
end

function utils.shallow_copy_table(t)
    local r = {}
    for k, v in pairs(t) do
        r[k] = v
    end
    return r
end

function utils.deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[utils.deepcopy(orig_key)] = utils.deepcopy(orig_value)
        end
        setmetatable(copy, utils.deepcopy(getmetatable(orig)))
    elseif torch.isTensor(orig) then
        copy = orig:clone()
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

function utils.add(t1, t2)
    assert(#t1 == #t2, "adding tables of different sizes, unsupported")
    for i=1, #t1 do
        t1[i] = t1[i] + t2[i]
    end
    return t1
end

function utils.mult(scalar, t)
    for i, v in pairs(t) do
        t[i] = v * scalar
    end
    return t
end

function utils.splitstr(inputstr, sep)
    if sep == nil then
        sep = "%s"
    end
    local t = {}
    local i = 1
    for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
        t[i] = str
        i = i + 1
    end
    return t
end

function utils.recursive_add(t1, t2)
    if t1 == nil then return t2 end
    if t2 == nil then return t1 end

    assert(type(t1) == type(t2))
    if type(t1) == "table" then
        local ret = {}
        for k, v in pairs(t1) do
            ret[k] = utils.recursive_add(v, t2[k])
        end
        for k, v in pairs(t2) do
            if ret[k] == nil then ret[k] = v end
        end
        return ret
    elseif type(t1) == "number" then
        return t1 + t2
    else
        error("Invalid types in recursive_add")
    end
end

function utils.recursive_div_c(t, c)
    if type(t) == "table" then
        local ret = {}
        for k, v in pairs(t) do
            ret[k] = utils.recursive_div_c(v, c)
        end
        return ret
    elseif type(t) == "number" then
        return t / c
    end
end

function utils.is_in(e, l)
    for _, v in pairs(l) do
        if e == v then
            return true
        end
    end
    return false
end

--[[
    * Modified from https://github.com/EmmanuelOga/columns/blob/master/utils/color.lua
    * Converts an HSV color value to RGB. Conversion formula
    * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
    * Assumes h, s, and v are contained in the set [0, 1] and
    * returns r, g, and b in the set [0, 255].
    *
    * @param   Number  h       The hue
    * @param   Number  s       The saturation
    * @param   Number  v       The value
    * @return  Array           The RGB representation
]]

function utils.hsv_to_rgb(hsv)
    local h, s, v = unpack(hsv)
    h = h / 360
    s = s / 100
    v = v / 100
    local r, g, b

    local i = math.floor(h * 6);
    local f = h * 6 - i;
    local p = v * (1 - s);
    local q = v * (1 - f * s);
    local t = v * (1 - (1 - f) * s);

    i = i % 6

    if i == 0 then r, g, b = v, t, p
    elseif i == 1 then r, g, b = q, v, p
    elseif i == 2 then r, g, b = p, v, t
    elseif i == 3 then r, g, b = p, q, v
    elseif i == 4 then r, g, b = t, p, v
    elseif i == 5 then r, g, b = v, p, q
    end

    local t = {math.floor(r * 255), math.floor(g * 255), math.floor(b * 255)}
    return t
end

function utils.get_health_color(hp_percentage, max_hp)
    -- 120 is green in HSV
    -- 0 -> 120 is roughly what standard BW uses
    hp_percentage = math.floor(hp_percentage / max_hp * 120)
    return utils.hsv_to_rgb({hp_percentage, 100, 100})
end

utils.visibility_color_table = {
    [0] = {0, 0, 0},       -- black
    [1] = {176,224,230},   -- powder blue
    [2] = {70,130,180},   -- steel blue
}

utils.players_color_table = {
    [0] = {255, 0, 0},    -- red
    [1] = {0, 0, 255},    -- blue
    [2] = {0, 128, 128},  -- teal
    [3] = {128, 0, 128},  -- purple
    [4] = {255,165,0},    -- orange
    [5] = {165,42,42},    -- brown
    [6] = {255,255,255},  -- white
    [7] = {255,255,0}     -- Yellow
}

-- TODO hash those later at some point instead of having a big table
utils.html_color_table = {
    [0] = {255,0,0},      -- Red
    [1] = {0,255,0},      -- Lime
    [2] = {0,0,255},      -- Blue
    [3] = {255,255,0},    -- Yellow
    [5] = {0,255,255},    -- Cyan / Aqua
    [7] = {255,0,255},    -- Magenta / Fuchsia
    [8] = {192,192,192},  -- Silver
    [9] = {128,128,128},  -- Gray
    [11] = {128,0,0},      -- Maroon
    [12] = {128,128,0},    -- Olive
    [13] = {0,128,0},      -- Green
    [14] = {128,0,128},    -- Purple
    [15] = {0,128,128},    -- Teal
    [30] = {0,0,128},      -- Navy
    [32] = {128,0,0},      -- maroon
    [33] = {139,0,0},      -- dark red
    [34] = {165,42,42},    -- brown
    [35] = {178,34,34},    -- firebrick
    [36] = {220,20,60},    -- crimson
    [37] = {255,0,0},      -- red
    [38] = {255,99,71},    -- tomato
    [39] = {255,127,80},   -- coral
    [40] = {205,92,92},    -- indian red
    [41] = {240,128,128},  -- light coral
    [42] = {233,150,122},  -- dark salmon
    [43] = {250,128,114},  -- salmon
    [44] = {255,160,122},  -- light salmon
    [45] = {255,69,0},     -- orange red
    [46] = {255,140,0},    -- dark orange
    [47] = {255,165,0},    -- orange
    [50] = {255,215,0},    -- gold
    [58] = {184,134,11},   -- dark golden rod
    [59] = {218,165,32},   -- golden rod
    [60] = {238,232,170},  -- pale golden rod
    [61] = {189,183,107},  -- dark khaki
    [62] = {240,230,140},  -- khaki
    [63] = {128,128,0},    -- olive
    [64] = {255,255,0},    -- yellow
    [65] = {154,205,50},   -- yellow green
    [66] = {85,107,47},    -- dark olive green
    [67] = {107,142,35},   -- olive drab
    [68] = {124,252,0},    -- lawn green
    [69] = {127,255,0},    -- chart reuse
    [70] = {173,255,47},   -- green yellow
    [71] = {0,100,0},      -- dark green
    [72] = {0,128,0},      -- green
    [73] = {34,139,34},    -- forest green
    [83] = {0,255,0},      -- lime
    [84] = {50,205,50},    -- lime green
    [85] = {144,238,144},  -- light green
    [89] = {152,251,152},  -- pale green
    [90] = {143,188,143},  -- dark sea green
    [93] = {0,250,154},    -- medium spring green
    [94] = {0,255,127},    -- spring green
    [95] = {46,139,87},    -- sea green
    [96] = {102,205,170},  -- medium aqua marine
    [97] = {60,179,113},   -- medium sea green
    [103] = {32,178,170},   -- light sea green
    [105] = {47,79,79},     -- dark slate gray
    [106] = {0,128,128},    -- teal
    [107] = {0,139,139},    -- dark cyan
    [108] = {0,255,255},    -- aqua
    [109] = {0,255,255},    -- cyan
    [110] = {224,255,255},  -- light cyan
    [111] = {0,206,209},    -- dark turquoise
    [112] = {64,224,208},   -- turquoise
    [113] = {72,209,204},   -- medium turquoise
    [114] = {175,238,238},  -- pale turquoise
    [115] = {127,255,212},  -- aqua marine
    [116] = {176,224,230},  -- powder blue
    [117] = {95,158,160},   -- cadet blue
    [118] = {70,130,180},   -- steel blue
    [120] = {100,149,237},  -- corn flower blue
    [122] = {0,191,255},    -- deep sky blue
    [123] = {30,144,255},   -- dodger blue
    [124] = {173,216,230},  -- light blue
    [125] = {135,206,235},  -- sky blue
    [130] = {135,206,250},  -- light sky blue
    [131] = {25,25,112},    -- midnight blue
    [132] = {0,0,128},      -- navy
    [133] = {0,0,139},      -- dark blue
    [134] = {0,0,205},      -- medium blue
    [135] = {0,0,255},      -- blue
    [136] = {65,105,225},   -- royal blue
    [137] = {138,43,226},   -- blue violet
    [138] = {75,0,130},     -- indigo
    [139] = {72,61,139},    -- dark slate blue
    [140] = {106,90,205},   -- slate blue
    [141] = {123,104,238},  -- medium slate blue
    [142] = {147,112,219},  -- medium purple
    [143] = {139,0,139},    -- dark magenta
    [144] = {148,0,211},    -- dark violet
    [146] = {153,50,204},   -- dark orchid
    [149] = {186,85,211},   -- medium orchid
    [154] = {128,0,128},    -- purple
    [155] = {216,191,216},  -- thistle
    [156] = {221,160,221},  -- plum
    [157] = {238,130,238},  -- violet
    [159] = {255,0,255},    -- magenta / fuchsia
    [160] = {218,112,214},  -- orchid
    [162] = {199,21,133},   -- medium violet red
    [163] = {219,112,147},  -- pale violet red
    [164] = {255,20,147},   -- deep pink
    [165] = {255,105,180},  -- hot pink
    [166] = {255,182,193},  -- light pink
    [167] = {255,192,203},  -- pink
    [169] = {250,235,215},  -- antique white
    [170] = {245,245,220},  -- beige
    [171] = {255,228,196},  -- bisque
    [172] = {255,235,205},  -- blanched almond
    [176] = {245,222,179},  -- wheat
    [177] = {255,248,220},  -- corn silk
    [178] = {255,250,205},  -- lemon chiffon
    [188] = {250,250,210},  -- light golden rod yellow
    [202] = {255,255,224},  -- light yellow
    [233] = {139,69,19}     -- saddle brown
    -- {160,82,45},    -- sienna
    -- {210,105,30},   -- chocolate
    -- {205,133,63},   -- peru
    -- {244,164,96},   -- sandy brown
    -- {222,184,135},  -- burly wood
    -- {210,180,140},  -- tan
    -- {188,143,143},  -- rosy brown
    -- {255,228,181},  -- moccasin
    -- {255,222,173},  -- navajo white
    -- {255,218,185},  -- peach puff
    -- {255,228,225},  -- misty rose
    -- {255,240,245},  -- lavender blush
    -- {250,240,230},  -- linen
    -- {253,245,230},  -- old lace
    -- {255,239,213},  -- papaya whip
    -- {255,245,238},  -- sea shell
    -- {245,255,250},  -- mint cream
    -- {112,128,144},  -- slate gray
    -- {119,136,153},  -- light slate gray
    -- {176,196,222},  -- light steel blue
    -- {230,230,250},  -- lavender
    -- {255,250,240},  -- floral white
    -- {240,248,255},  -- alice blue
    -- {248,248,255},  -- ghost white
    -- {240,255,240},  -- honeydew
    -- {255,255,240},  -- ivory
    -- {240,255,255},  -- azure
    -- {255,250,250},  -- snow
    -- {105,105,105},  -- dim gray / dim grey
    -- {128,128,128},  -- gray / grey
    -- {169,169,169},  -- dark gray / dark grey
    -- {192,192,192},  -- silver
    -- {211,211,211},  -- light gray / light grey
    -- {220,220,220},  -- gainsboro
    -- {245,245,245},  -- white smoke
    -- {255,255,255}  -- white
}

return utils
