local OverlayLayer = {
    name = "TestLayerB"
}

local g = helios.graphics
local key = helios.key

local function color(r, g_value, b, a)
    return { r = r, g = g_value, b = b, a = a or 255 }
end

function OverlayLayer:on_attach()
    helios.log.info("Layer B Attached!")
end

function OverlayLayer:on_detach()
    helios.log.info("Layer B detached")
end

function OverlayLayer:draw()
    local width = helios.window.width()
    local height = helios.window.height()
    local center_x = width * 0.5
    local center_y = height * 0.5

    g.rectangle(0, 0, width, height, g.fade(color(18, 24, 48, 255), 0.72))

    g.circle(center_x - 190.0, center_y - 98.0, 70.0, g.fade(color(255, 80, 140, 255), 0.86))
    g.triangle(
        center_x + 168.0,
        center_y - 168.0,
        center_x + 256.0,
        center_y - 28.0,
        center_x + 80.0,
        center_y - 28.0,
        g.fade(color(115, 232, 255, 255), 0.9)
    )

    local panel_x = center_x - 260.0
    local panel_y = center_y - 92.0
    local panel_w = 520.0
    local panel_h = 184.0

    g.rectangle(panel_x, panel_y, panel_w, panel_h, color(246, 248, 255, 242))
    g.rectangle_lines(panel_x, panel_y, panel_w, panel_h, 5.0, color(78, 93, 199, 255))

    g.text(self.name, panel_x + 34.0, panel_y + 28.0, 46, color(45, 54, 134, 255))
    g.text("Overlay layer: arrow keys log input", panel_x + 36.0, panel_y + 88.0, 22, color(68, 75, 118, 255))
    g.text("Release SPACE to remove this overlay", panel_x + 36.0, panel_y + 120.0, 22, color(68, 75, 118, 255))

    g.circle_lines(center_x - 120.0, center_y + 144.0, 48.0, color(255, 222, 89, 255))
    g.rectangle(center_x + 86.0, center_y + 116.0, 112.0, 58.0, color(255, 222, 89, 255))
end

function OverlayLayer:on_key_held(event)
    if event.key == key.up or event.key == key.down or event.key == key.left or event.key == key.right then
        event.handled = true
    end
end

function OverlayLayer:on_key_pressed(event)
    if event.key == key.up then
        helios.log.info("^")
        event.handled = true
    elseif event.key == key.down then
        helios.log.info("v")
        event.handled = true
    elseif event.key == key.left then
        helios.log.info("<")
        event.handled = true
    elseif event.key == key.right then
        helios.log.info(">")
        event.handled = true
    end
end

return OverlayLayer
