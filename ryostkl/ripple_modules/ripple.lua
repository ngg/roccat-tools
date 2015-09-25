-- This file is part of roccat-tools.
--
-- roccat-tools is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 2 of the License, or
-- (at your option) any later version.
--
-- roccat-tools is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.

-- Implements default ripple effect.
-- Differences to Roccat implementation:
-- Roccat uses a vague position table and costly floating point arithmetics for
-- drawing a circle.
-- I'm using a fine grain position table and a cheap integer algorithm for drawing.

-- table ryosmk has following functions defined:
-- set_key(arg)   Takes hid usage id as integer or table with integers as keys (set).
--                Switches the key on.
-- clear_key(arg) Takes hid usage id as integer or table with integers as keys (set).
--                Switches the key off.
-- get_key(hid)   Returns the boolean state of the integer hid usage id.
-- clear()        switches all keys off.
-- empty()        returns true if all keys are off, else false.

-- Constants.
local MATRIX_ROWS = 25
local MATRIX_COLS = 95
local KEY_WIDTH = 4

-- Values for state.
local center
local radius
local leds

-- Maps a hid usage id to a x,y position on the keyboard.
local hid_to_light_position = {
  -- thumbster keys don't send events

  -- keys_function
  [0x3a]={x=10, y= 2}, [0x3b]={x=14, y= 2}, [0x3c]={x=18, y= 2}, [0x3d]={x=22, y= 2},
  [0x3e]={x=28, y= 2}, [0x3f]={x=32, y= 2}, [0x40]={x=36, y= 2}, [0x41]={x=40, y= 2},
  [0x42]={x=46, y= 2}, [0x43]={x=50, y= 2}, [0x44]={x=54, y= 2}, [0x45]={x=58, y= 2},
  [0x46]={x=63, y= 2}, [0x47]={x=67, y= 2}, [0x48]={x=71, y= 2},

  -- keys_easyzone
  [0x35]={x= 2, y= 7}, [0x1e]={x= 6, y= 7}, [0x1f]={x=10, y= 7}, [0x20]={x=14, y= 7},
  [0x21]={x=18, y= 7}, [0x22]={x=22, y= 7}, [0x23]={x=26, y= 7}, [0x24]={x=30, y= 7},
  [0x25]={x=34, y= 7}, [0x26]={x=38, y= 7}, [0x27]={x=42, y= 7}, [0x2d]={x=46, y= 7},
  [0x2e]={x=50, y= 7}, [0x89]={x=54, y= 7}, [0x2a]={x=58, y= 7}, [0x49]={x=63, y= 7},
  [0x4a]={x=67, y= 7}, [0x4b]={x=71, y= 7}, [0x2b]={x= 3, y=11}, [0x14]={x= 8, y=11},
  [0x1a]={x=12, y=11}, [0x08]={x=16, y=11}, [0x15]={x=20, y=11}, [0x17]={x=24, y=11},
  [0x1c]={x=28, y=11}, [0x18]={x=32, y=11}, [0x0c]={x=36, y=11}, [0x12]={x=40, y=11},
  [0x13]={x=44, y=11}, [0x2f]={x=48, y=11}, [0x30]={x=52, y=11}, [0x31]={x=57, y=11},
  [0x4c]={x=63, y=11}, [0x4d]={x=67, y=11}, [0x4e]={x=71, y=11}, [0x04]={x= 9, y=15},
  [0x16]={x=13, y=15}, [0x07]={x=17, y=15}, [0x09]={x=21, y=15}, [0x0a]={x=25, y=15},
  [0x0b]={x=29, y=15}, [0x0d]={x=33, y=15}, [0x0e]={x=37, y=15}, [0x0f]={x=41, y=15},
  [0x33]={x=45, y=15}, [0x34]={x=49, y=15}, [0x32]={x=53, y=15}, [0x28]={x=57, y=15},
  [0xe1]={x= 3, y=19}, [0x64]={x= 7, y=19}, [0x1d]={x=11, y=19}, [0x1b]={x=15, y=19},
  [0x06]={x=19, y=19}, [0x19]={x=23, y=19}, [0x05]={x=27, y=19}, [0x11]={x=31, y=19},
  [0x10]={x=35, y=19}, [0x36]={x=39, y=19}, [0x37]={x=43, y=19}, [0x38]={x=47, y=19},
  [0x87]={x=51, y=19}, [0xe5]={x=56, y=19}, [0x52]={x=67, y=19}, [0xe0]={x= 3, y=23},
  [0xe3]={x= 8, y=23}, [0xe2]={x=13, y=23}, [0x8b]={x=17, y=23}, [0x2c]={x=26, y=23},
  [0x8a]={x=35, y=23}, [0x88]={x=39, y=23}, [0xe6]={x=44, y=23}, [0x65]={x=52, y=23},
  [0xe4]={x=57, y=23}, [0x50]={x=63, y=23}, [0x51]={x=67, y=23}, [0x4f]={x=71, y=23},

  -- keys_extra
  [0x39]={x= 4, y=15}, [0xf1]={x=48, y=23}, [0x29]={x= 2, y= 2},
}

local light_position_matrix = {}

-- Sets corresponding hid usage id in set if any.
local function set_led(leds, x, y)
  if y >= MATRIX_ROWS or y < 0 or x >= MATRIX_COLS or x < 0 then
    return
  end

  hid = light_position_matrix[x + y * MATRIX_COLS]
  if hid then
    leds[hid] = true
  end
end

-- QUOTE Midpoint circle algorithm inspired by Perone's programming pad
-- http://www.willperone.net/Code/codecircle.php
-- Copyright (c) Will Perone
--
-- Returns set of hid usage ids to set/clear.
local function draw_circle (center, radius)
  local radius = radius * KEY_WIDTH
  local leds = {}
  
  if radius < 1 then
    return leds
  end
  
  local x = 0
  local y = radius
  local d = 1 - radius
  local delta_e = 3
  local delta_se = 5 - radius * 2

  set_led(leds, center.x,     center.y - y)
  set_led(leds, center.x,     center.y + y)
  set_led(leds, center.x - y, center.y    )
  set_led(leds, center.x + y, center.y    )
  
  while y > x do
    if d < 0 then
      d = d + delta_e
      delta_se = delta_se + 2
    else
      d = d + delta_se
      delta_se = delta_se + 4
      y = y - 1
    end
    delta_e = delta_e + 2
    x = x + 1
    
    set_led(leds, center.x - x, center.y - y)
    set_led(leds, center.x - y, center.y - x)
    set_led(leds, center.x + y, center.y - x)
    set_led(leds, center.x + x, center.y - y)
    set_led(leds, center.x - x, center.y + y)
    set_led(leds, center.x - y, center.y + x)
    set_led(leds, center.x + y, center.y + x)
    set_led(leds, center.x + x, center.y + y)
  end

  return leds
end

-- Function start() gets called when a key is pressed and receives hid usage id
-- of pressed key and returns nothing.
function start (hid)
  center = hid_to_light_position[hid]
  radius = 1
  leds = nil
end

-- Function tick() gets called repeatedly and should return waiting time in msec
-- until next tick or nil if done.
-- One tick: draw new outer circle
-- Next tick: remove inner circle
-- Next tick: draw new outer circle
function tick ()
  if leds then
    ryosmk.clear_key(leds)
    leds = nil
    return 0
  else
    leds = draw_circle(center, radius)
    ryosmk.set_key(leds)
    radius = radius + 1
  
    if ryosmk.empty() then
      return nil
    else
      return 50
    end
  end
end

-- Execution starts here on load.

-- Creating a matrix of led positions.
for hid, position in pairs(hid_to_light_position) do
  local base = position.x - 2 + (position.y - 2) * MATRIX_COLS
  
  light_position_matrix[base + 1] = hid
  light_position_matrix[base + 2] = hid

  base = base + MATRIX_COLS
  
  light_position_matrix[base    ] = hid
  light_position_matrix[base + 1] = hid
  light_position_matrix[base + 2] = hid
  light_position_matrix[base + 3] = hid

  base = base + MATRIX_COLS
  
  light_position_matrix[base    ] = hid
  light_position_matrix[base + 1] = hid
  light_position_matrix[base + 2] = hid
  light_position_matrix[base + 3] = hid

  base = base + MATRIX_COLS
  
  light_position_matrix[base + 1] = hid
  light_position_matrix[base + 2] = hid
end
