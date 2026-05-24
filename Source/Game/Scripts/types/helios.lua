---@meta

---@class LayerHandle
local LayerHandle = {}

---@return boolean
function LayerHandle:is_valid() end

---@return integer
function LayerHandle:id() end

---@class Entity
local Entity = {}

---@return boolean
function Entity:is_valid() end

function Entity:destroy() end

---@param component "position"|"velocity"|"acceleration"|"radius"|"color"
---@param values table
---@return PositionComponent|VelocityComponent|AccelerationComponent|RadiusComponent|ColorComponent|nil
function Entity:add(component, values) end

---@param component "position"|"velocity"|"acceleration"|"radius"|"color"
---@param values table
---@return PositionComponent|VelocityComponent|AccelerationComponent|RadiusComponent|ColorComponent|nil
function Entity:replace(component, values) end

---@param component "position"|"velocity"|"acceleration"|"radius"|"color"
---@return PositionComponent|VelocityComponent|AccelerationComponent|RadiusComponent|ColorComponent|nil
function Entity:get(component) end

---@param component "position"|"velocity"|"acceleration"|"radius"|"color"
---@return boolean
function Entity:has(component) end

---@param component "position"|"velocity"|"acceleration"|"radius"|"color"
---@return boolean
function Entity:remove(component) end

---@class PositionComponent
---@field x number
---@field y number

---@class VelocityComponent
---@field x number
---@field y number

---@class AccelerationComponent
---@field x number
---@field y number

---@class RadiusComponent
---@field value number

---@class ColorComponent
---@field r integer
---@field g integer
---@field b integer
---@field a integer

---@class HeliosColor
---@field r integer
---@field g integer
---@field b integer
---@field a integer

---@class KeyEvent
---@field key integer
---@field repeat? boolean
---@field handled boolean

---@class MouseButtonEvent
---@field button integer
---@field handled boolean

---@class MouseMovedEvent
---@field x number
---@field y number
---@field delta_x number
---@field delta_y number
---@field handled boolean

---@class MouseScrolledEvent
---@field offset_x number
---@field offset_y number
---@field handled boolean

---@class WindowResizedEvent
---@field width integer
---@field height integer
---@field handled boolean

---@class LuaLayer
---@field name? string
---@field on_attach? fun(self: LuaLayer)
---@field on_detach? fun(self: LuaLayer)
---@field update? fun(self: LuaLayer, dt: number)
---@field draw? fun(self: LuaLayer)
---@field on_key_held? fun(self: LuaLayer, event: KeyEvent)
---@field on_key_pressed? fun(self: LuaLayer, event: KeyEvent)
---@field on_key_released? fun(self: LuaLayer, event: KeyEvent)
---@field on_mouse_button_pressed? fun(self: LuaLayer, event: MouseButtonEvent)
---@field on_mouse_button_released? fun(self: LuaLayer, event: MouseButtonEvent)
---@field on_mouse_moved? fun(self: LuaLayer, event: MouseMovedEvent)
---@field on_mouse_scrolled? fun(self: LuaLayer, event: MouseScrolledEvent)
---@field on_window_resized? fun(self: LuaLayer, event: WindowResizedEvent)

---@class Helios
---@field layers HeliosLayers
---@field world HeliosWorld
---@field graphics HeliosGraphics
---@field window HeliosWindow
---@field input HeliosInput
---@field random HeliosRandom
---@field log HeliosLog
---@field native HeliosNative
---@field key table<string, integer>
---@field mouse table<string, integer>
helios = {}

---@class HeliosLayers
helios.layers = {}

---@param layer LuaLayer|fun(): LuaLayer
---@return LayerHandle
function helios.layers.push(layer) end

---@param handle LayerHandle
---@return boolean
function helios.layers.remove(handle) end

function helios.layers.pop() end

---@param handle LayerHandle
---@return boolean
function helios.layers.is_valid(handle) end

---@class HeliosWorld
helios.world = {}

---@return Entity
function helios.world:create() end

---@param components string[]
---@param callback fun(entity: Entity, ...: any)
function helios.world:each(components, callback) end

---@class HeliosGraphics
helios.graphics = {}

---@param x number
---@param y number
---@param width number
---@param height number
---@param color HeliosColor
function helios.graphics.rectangle(x, y, width, height, color) end

---@param x number
---@param y number
---@param width number
---@param height number
---@param top_color HeliosColor
---@param bottom_color HeliosColor
function helios.graphics.rectangle_gradient_v(x, y, width, height, top_color, bottom_color) end

---@param x number
---@param y number
---@param width number
---@param height number
---@param line_thick number
---@param color HeliosColor
function helios.graphics.rectangle_lines(x, y, width, height, line_thick, color) end

---@param x number
---@param y number
---@param radius number
---@param color HeliosColor
function helios.graphics.circle(x, y, radius, color) end

---@param x number
---@param y number
---@param radius number
---@param color HeliosColor
function helios.graphics.circle_lines(x, y, radius, color) end

---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@param x3 number
---@param y3 number
---@param color HeliosColor
function helios.graphics.triangle(x1, y1, x2, y2, x3, y3, color) end

---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@param color HeliosColor
function helios.graphics.line(x1, y1, x2, y2, color) end

---@param text string
---@param x number
---@param y number
---@param font_size integer
---@param color HeliosColor
function helios.graphics.text(text, x, y, font_size, color) end

---@param text string
---@param font_size integer
---@return integer
function helios.graphics.measure_text(text, font_size) end

---@param color HeliosColor
---@param alpha number
---@return HeliosColor
function helios.graphics.fade(color, alpha) end

---@class HeliosWindow
helios.window = {}

---@return integer
function helios.window.width() end

---@return integer
function helios.window.height() end

---@class HeliosInput
helios.input = {}

---@return { x: number, y: number }
function helios.input.mouse_position() end

---@class HeliosRandom
helios.random = {}

---@param min integer
---@param max integer
---@return integer
function helios.random.int(min, max) end

---@class HeliosLog
helios.log = {}

function helios.log.info(...) end
function helios.log.warn(...) end
function helios.log.error(...) end

---@class HeliosNative
helios.native = {}

---Runs a registered C++ system. The sample ball layer uses "game.balls.physics".
---@param name string
---@param ... unknown
---@return boolean
function helios.native.run(name, ...) end
