local OverlayLayer = require("layers.overlay")

local BallsLayer = {
    name = "TestLayerA",
    test_layer_b = nil,
    dragged_ball = nil,
    drag_offset_x = 0.0,
    drag_offset_y = 0.0,
    ball_action_timer = 0.0
}

local g = helios.graphics
local key = helios.key
local mouse = helios.mouse

local gravity = 420.0
local ball_action_interval = 0.035
local drag_velocity_scale = 18.0
local ball_physics_system = "game.balls.physics"

local function color(r, g_value, b, a)
    return { r = r, g = g_value, b = b, a = a or 255 }
end

local function component_color(component)
    return { r = component.r, g = component.g, b = component.b, a = component.a }
end

local function valid_entity(entity)
    return entity ~= nil and entity:is_valid()
end

local function valid_layer(handle)
    return handle ~= nil and helios.layers.is_valid(handle)
end

function BallsLayer:on_attach()
    helios.log.info("Layer A Attached!")
end

function BallsLayer:on_detach()
    helios.log.info("Layer A detached")
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

    for x = 0, width, 64 do
        g.line(x, 0, x, height, g.fade(skyblue, 0.25))
    end

    for y = 0, height, 64 do
        g.line(0, y, width, y, g.fade(skyblue, 0.18))
    end

    local ball_count = 0

    helios.world:each({ "position", "color", "radius" }, function(_, position, ball_color, radius)
        g.circle(position.x, position.y, radius.value, component_color(ball_color))
        ball_count = ball_count + 1
    end)

    local ball_count_text = string.format("Balls: %d", ball_count)
    local font_size = 26
    local text_width = g.measure_text(ball_count_text, font_size)

    g.text(
        ball_count_text,
        (width - text_width) / 2,
        24,
        font_size,
        color(35, 82, 100, 255)
    )
end

function BallsLayer:spawn_random_ball()
    local radius = helios.random.int(10, 32)
    local width = helios.window.width()
    local height = helios.window.height()
    local x = helios.random.int(radius, width - radius)
    local y = helios.random.int(radius, math.floor(height / 2))
    local velocity_x = helios.random.int(-280, 280)
    local velocity_y = helios.random.int(-260, 40)

    local ball = helios.world:create()
    ball:add("position", { x = x, y = y })
    ball:add("radius", { value = radius })
    ball:add("velocity", { x = velocity_x, y = velocity_y })
    ball:add("acceleration", { x = 0.0, y = gravity })
    ball:add("color", {
        r = helios.random.int(64, 255),
        g = helios.random.int(64, 255),
        b = helios.random.int(64, 255),
        a = 255
    })
end

function BallsLayer:remove_ball()
    local ball_to_remove = nil

    helios.world:each({ "position", "color", "radius" }, function(entity)
        if not valid_entity(ball_to_remove) then
            ball_to_remove = entity
        end
    end)

    if valid_entity(ball_to_remove) then
        ball_to_remove:destroy()
    end
end

function BallsLayer:start_dragging_ball(mouse_x, mouse_y)
    self.dragged_ball = nil
    self.drag_offset_x = 0.0
    self.drag_offset_y = 0.0

    local best_distance_squared = 0.0

    helios.world:each({ "position", "velocity", "radius" }, function(entity, position, _, radius)
        local delta_x = mouse_x - position.x
        local delta_y = mouse_y - position.y
        local distance_squared = delta_x * delta_x + delta_y * delta_y
        local radius_squared = radius.value * radius.value

        if distance_squared > radius_squared then
            return
        end

        if valid_entity(self.dragged_ball) and distance_squared >= best_distance_squared then
            return
        end

        self.dragged_ball = entity
        self.drag_offset_x = position.x - mouse_x
        self.drag_offset_y = position.y - mouse_y
        best_distance_squared = distance_squared
    end)
end

function BallsLayer:drag_ball(mouse_x, mouse_y, delta_x, delta_y)
    if not valid_entity(self.dragged_ball) then
        return
    end

    local position = self.dragged_ball:get("position")
    local velocity = self.dragged_ball:get("velocity")

    if position == nil or velocity == nil then
        self:stop_dragging_ball()
        return
    end

    position.x = mouse_x + self.drag_offset_x
    position.y = mouse_y + self.drag_offset_y
    velocity.x = delta_x * drag_velocity_scale
    velocity.y = delta_y * drag_velocity_scale
end

function BallsLayer:stop_dragging_ball()
    self.dragged_ball = nil
    self.drag_offset_x = 0.0
    self.drag_offset_y = 0.0
end

function BallsLayer:on_key_event(event)
    if event.action == "held" then
        if self.ball_action_timer < ball_action_interval then
            return
        end

        if event.key == key.up then
            self:spawn_random_ball()
            self.ball_action_timer = 0.0
            event.handled = true
        elseif event.key == key.down then
            self:remove_ball()
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

    if event.key == key.w then
        helios.log.info("w")
        event.handled = true
    elseif event.key == key.a then
        helios.log.info("a")
        event.handled = true
    elseif event.key == key.s then
        helios.log.info("s")
        event.handled = true
    elseif event.key == key.d then
        helios.log.info("d")
        event.handled = true
    elseif event.key == key.space then
        if not valid_layer(self.test_layer_b) then
            self.test_layer_b = helios.layers.push(OverlayLayer)
        end

        event.handled = true
    elseif event.key == key.down then
        self:remove_ball()
        event.handled = true
    end
end

function BallsLayer:on_mouse_event(event)
    if event.action == "button_pressed" and event.button == mouse.left then
        local position = helios.input.mouse_position()
        self:start_dragging_ball(position.x, position.y)
        event.handled = valid_entity(self.dragged_ball)
        return
    end

    if event.action == "button_released" and event.button == mouse.left and valid_entity(self.dragged_ball) then
        self:stop_dragging_ball()
        event.handled = true
        return
    end

    if event.action ~= "moved" or not valid_entity(self.dragged_ball) then
        return
    end

    self:drag_ball(event.x, event.y, event.delta_x, event.delta_y)
    event.handled = true
end

return BallsLayer
