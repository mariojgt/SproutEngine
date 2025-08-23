-- Minimal example script for SproutEngine
-- API available in C++ binding: GetRotation(id) / SetRotation(id, vec3), Print(msg)

speed = speed or 45.0 -- deg/sec; can be overridden by inspector later

function OnStart(id)
  Print("Rotate.lua OnStart for entity " .. tostring(id))
end

function OnTick(id, dt)
  local x, y, z = GetRotation(id)
  y = y + speed * dt
  SetRotation(id, {x, y, z})
end
