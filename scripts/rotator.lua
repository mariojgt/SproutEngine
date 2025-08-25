\
-- Demo: spins the entity around Y axis and bobs up/down
speed = 45.0  -- degrees per second
t = 0.0

function OnStart()
    -- nothing yet
end

function OnUpdate(dt)
    t = t + dt
    rotate_y_degrees(speed * dt)
    local x,y,z = get_position()
    y = 0.25 * math.sin(t * 2.0)
    set_position(x, y, z)
end
