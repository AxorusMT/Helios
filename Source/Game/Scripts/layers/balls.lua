local OverlayLayer = require("layers.overlay")

local BallsLayer = {
    name = "LuaBallsLayer",
    test_layer_b = nil,
    ball_action_timer = 0.0,

    draw_outline = false
}

local g = helios.graphics
local key = helios.key
local mouse = helios.mouse

local ball_action_interval = 0.035
local ball_physics_system = "game.balls.physics"
local ball_draw_system = "game.balls.draw"
local ball_spawn_random_system = "game.balls.spawn_random"
local ball_spawn_cursor_system = "game.balls.spawn_cursor"
local ball_remove_system = "game.balls.remove"
local ball_pulse_system = "game.balls.pulse"

local function color(r, g_value, b, a)
    return { r = r, g = g_value, b = b, a = a or 255 }
end

local function valid_layer(handle)
    return handle ~= nil and helios.layers.is_valid(handle)
end

function BallsLayer:on_attach()
    helios.log.info("Lua balls layer attached")
end

function BallsLayer:on_detach()
    helios.log.info("Lua balls layer detached")
end

function BallsLayer:update(dt)
    self.ball_action_timer = self.ball_action_timer + dt
    helios.native.run(ball_physics_system, dt)
end

function BallsLayer:draw()
    local width = helios.window.width()
    local height = helios.window.height()

    g.rectangle_gradient_v(
        0,
        0,
        width,
        height,
        color(220, 242, 255, 255),
        color(218, 248, 224, 255)
    )

    local skyblue = color(102, 191, 255, 255)
    local grid_line_color = g.fade(skyblue, 0.25)
    local grid_row_color = g.fade(skyblue, 0.18)

    for x = 0, width, 64 do
        g.line(x, 0, x, height, grid_line_color)
    end

    for y = 0, height, 64 do
        g.line(0, y, width, y, grid_row_color)
    end

    helios.native.run(ball_draw_system, self.draw_outline)
end

function BallsLayer:on_key_event(event)
    if event.action == "held" then
        if self.ball_action_timer < ball_action_interval then
            return
        end

        if event.key == key.up then
            helios.native.run(ball_spawn_random_system, 1)
            self.ball_action_timer = 0.0
            event.handled = true
        elseif event.key == key.down then
            helios.native.run(ball_remove_system, 1)
            self.ball_action_timer = 0.0
            event.handled = true
        elseif event.key == key.v then
            helios.native.run(ball_spawn_random_system, 100)
            self.ball_action_timer = 0.0
            event.handled = true
        elseif event.key == key.c then
            helios.native.run(ball_remove_system, 100)
            self.ball_action_timer = 0.0
            event.handled = true
        end

        return
    end

    if event.action == "released" then
        if event.key ~= key.space or not valid_layer(self.test_layer_b) then
            return
        end

        helios.layers.remove(self.test_layer_b)
        self.test_layer_b = nil
        event.handled = true
        return
    end

    if event.action ~= "pressed" then
        return
    end

    if event.key == key.b then
        helios.native.run(ball_spawn_cursor_system, 100.0)
        event.handled = true
    elseif event.key == key.c then
        helios.native.run(ball_remove_system, 100)
        event.handled = true
    elseif event.key == key.v then
        helios.native.run(ball_spawn_random_system, 100)
        event.handled = true
    elseif event.key == key.o then
        self.draw_outline = not self.draw_outline
        helios.log.info("outline 67 ", self.draw_outline)
        event.handled = true
    elseif event.key == key.space then
        if not valid_layer(self.test_layer_b) then
            self.test_layer_b = helios.layers.push(OverlayLayer)
        end

        event.handled = true
    elseif event.key == key.down then
        helios.native.run(ball_remove_system, 1)
        event.handled = true
    end
end

function BallsLayer:on_mouse_event(event)
    if event.action == "button_pressed" and event.button == mouse.left then
        helios.native.run(ball_pulse_system)
        event.handled = true
        return
    end
end

return BallsLayer
