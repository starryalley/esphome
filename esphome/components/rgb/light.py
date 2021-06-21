import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.const import CONF_BLUE, CONF_GREEN, CONF_RED, CONF_OUTPUT_ID

rgb_ns = cg.esphome_ns.namespace("rgb")
RGBLightOutput = rgb_ns.class_("RGBLightOutput", light.LightOutput)

RGBLightOutputAddressable = rgb_ns.class_(
    "RGBLightOutputAddressable", light.AddressableLight
)

CONF_ADDRESSABLE_PIXEL = "addressable_pixel"

if config[CONF_ADDRESSABLE_PIXEL]:
    CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(RGBLightOutput),
            cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
            cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
            cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
            cv.Optional(CONF_ADDRESSABLE_PIXEL, default=False): cv.boolean,
        }
    )
else:
    CONFIG_SCHEMA = light.ADDRESSABLE_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(RGBLightOutput),
            cv.Required(CONF_RED): cv.use_id(output.FloatOutput),
            cv.Required(CONF_GREEN): cv.use_id(output.FloatOutput),
            cv.Required(CONF_BLUE): cv.use_id(output.FloatOutput),
            cv.Optional(CONF_ADDRESSABLE_PIXEL, default=False): cv.boolean,
        }
    )


async def to_code(config):
    #template = cg.TemplateArguments(getattr(cg.global_ns, format_method(config)))
    if config[CONF_ADDRESSABLE_PIXEL]:
        #out_type = NeoPixelRGBWLightOutput.template(template)
        out_type = RGBLightOutputAddressable
    else:
        #out_type = NeoPixelRGBLightOutput.template(template)
        out_type = RGBLightOutput


    rhs = out_type.new()
    var = cg.Pvariable(config[CONF_OUTPUT_ID], rhs, out_type)

    #var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)

    red = await cg.get_variable(config[CONF_RED])
    cg.add(var.set_red(red))
    green = await cg.get_variable(config[CONF_GREEN])
    cg.add(var.set_green(green))
    blue = await cg.get_variable(config[CONF_BLUE])
    cg.add(var.set_blue(blue))
