local BallsLayer = require("layers.balls")

local StartLayer = {
    name = "StartLayer",
    started = false,
    selected_button = nil
}

local g = helios.graphics
local key = helios.key
local mouse = helios.mouse

local function color(r, g_value, b, a)
    return { r = r, g = g_value, b = b, a = a or 255 }
end

local function point_in_rect(point, rect)
    return point.x >= rect.x
        and point.x <= rect.x + rect.w
        and point.y >= rect.y
        and point.y <= rect.y + rect.h
end

local function draw_centered_text(text, center_x, y, font_size, text_color)
    local text_width = g.measure_text(text, font_size)
    g.text(text, center_x - text_width * 0.5, y, font_size, text_color)
end

local function draw_button(rect, label, hovered, primary)
    local fill = primary and color(28, 61, 78, 255) or color(238, 247, 250, 240)
    local line = primary and color(255, 222, 89, 255) or color(52, 99, 118, 255)
    local text = primary and color(245, 252, 255, 255) or color(28, 61, 78, 255)

    if hovered then
        fill = primary and color(36, 82, 102, 255) or color(255, 252, 236, 255)
        line = color(255, 222, 89, 255)
    end

    g.rectangle(rect.x, rect.y, rect.w, rect.h, fill)
    g.rectangle_lines(rect.x, rect.y, rect.w, rect.h, hovered and 4.0 or 2.0, line)
    draw_centered_text(label, rect.x + rect.w * 0.5, rect.y + 17.0, 26, text)
end

function StartLayer:on_attach()
    helios.log.info("Start screen attached")
end

function StartLayer:layout()
    local width = helios.window.width()
    local height = helios.window.height()
    local center_x = width * 0.5
    local center_y = height * 0.5
    local button_w = 240.0
    local button_h = 62.0

    return {
        center_x = center_x,
        center_y = center_y,
        start = {
            x = center_x - button_w - 18.0,
            y = center_y + 118.0,
            w = button_w,
            h = button_h
        },
        quit = {
            x = center_x + 18.0,
            y = center_y + 118.0,
            w = button_w,
            h = button_h
        }
    }
end

function StartLayer:start_game()
    if self.started then
        return
    end

    self.started = true
    helios.layers.push(BallsLayer)
end

function StartLayer:draw()
    if self.started then
        return
    end

    local width = helios.window.width()
    local height = helios.window.height()
    local layout = self:layout()
    local cursor = helios.input.mouse_position()
    local start_hovered = point_in_rect(cursor, layout.start)
    local quit_hovered = point_in_rect(cursor, layout.quit)

    g.rectangle_gradient_v(
        0,
        0,
        width,
        height,
        color(212, 239, 247, 255),
        color(225, 250, 228, 255)
    )

    for x = -64, width + 64, 96 do
        g.line(x, 0, x + 160, height, g.fade(color(64, 161, 185, 255), 0.12))
    end

    g.circle(layout.center_x + 350.0, layout.center_y - 150.0, 88.0, g.fade(color(255, 222, 89, 255), 0.72))
    g.circle(layout.center_x - 330.0, layout.center_y + 72.0, 58.0, g.fade(color(99, 219, 255, 255), 0.7))
    g.circle(layout.center_x + 252.0, layout.center_y + 34.0, 28.0, g.fade(color(36, 82, 102, 255), 0.55))

    draw_centered_text("HELIOS CATCH", layout.center_x, layout.center_y - 188.0, 64, color(22, 55, 70, 255))
    draw_centered_text(
        "Catch the falling balls. Chain catches, spend pulses, survive the waves.",
        layout.center_x,
        layout.center_y - 102.0,
        24,
        color(44, 85, 102, 255)
    )

    draw_centered_text("Mouse or A/D moves the catcher", layout.center_x, layout.center_y - 28.0, 24, color(31, 70, 86, 255))
    draw_centered_text("Left click or X fires a pulse", layout.center_x, layout.center_y + 8.0, 24, color(31, 70, 86, 255))
    draw_centered_text("P pauses. R restarts.", layout.center_x, layout.center_y + 44.0, 24, color(31, 70, 86, 255))

    draw_button(layout.start, "Start", start_hovered, true)
    draw_button(layout.quit, "Quit", quit_hovered, false)
end

function StartLayer:on_key_event(event)
    if self.started or event.action ~= "pressed" then
        return
    end

    if event.key == key.space then
        self:start_game()
        event.handled = true
    end
end

function StartLayer:on_mouse_event(event)
    if self.started or event.action ~= "button_pressed" or event.button ~= mouse.left then
        return
    end

    local cursor = helios.input.mouse_position()
    local layout = self:layout()

    if point_in_rect(cursor, layout.start) then
        self:start_game()
        event.handled = true
    elseif point_in_rect(cursor, layout.quit) then
        helios.window.quit()
        event.handled = true
    end
end

return StartLayer
