import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import automation
from esphome.components import i2c, time
from esphome.const import CONF_ID


CODEOWNERS = ["@WeekendWarrior"]
DEPENDENCIES = ["i2c"]
at24c_ns = cg.esphome_ns.namespace("at24c")
AT24CComponent = at24c_ns.class_("AT24CComponent", cg.Component, i2c.I2CDevice)
WriteAction = at24c_ns.class_("WriteAction", automation.Action)
ReadAction = at24c_ns.class_("ReadAction", automation.Action)
DumpAction = at24c_ns.class_("DumpAction", automation.Action)


CONFIG_SCHEMA = time.TIME_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(AT24CComponent),
    }
).extend(i2c.i2c_device_schema(0x57))


@automation.register_action(
    "at24c.write_data",
    WriteAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(AT24CComponent),
            # optional time component to attach timestamp
            # cv.GenerateID(): cv.use_id(AT24CComponent),
            # data to be written (could be float, int, etc)
        }
    ),
)
async def at24c_write_data_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "at24c.read_data",
    ReadAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(AT24CComponent),
            #address?
        }
    ),
)
async def at24c_read_data_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var

@automation.dump_data(
    "at24c.dump_data",
    ReadAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(AT24CComponent),
            #address range?
            #serial output?
            # hass api?
            # mqtt api?
            # can you publish data with custom timestamps?
        }
    ),
)
async def at24c_dump_data_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await time.register_time(var, config)
