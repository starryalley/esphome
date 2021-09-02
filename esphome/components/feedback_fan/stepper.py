import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import stepper, hbridge, pulse_meter, sensor
from esphome.const import (
    CONF_ID,
    CONF_SENSOR_ID,
)


feedback_fan_ns = cg.esphome_ns.namespace("feedback_fan")
FeedbackFan = feedback_fan_ns.class_("FeedbackFan", stepper.Stepper, cg.Component)

SetInitialOutputAction = feedback_fan_ns.class_(
    "SetInitialOutputAction", automation.Action
)

# validate to make sure it's using at least one sensor and one fan
CONF_FAN_ID = "fan_id"
CONF_HBRIDGE_ID = "hbridge_fan_id"
CONF_INITIAL_OUTPUT = "initial_output"
CONF_MIN_OUTPUT = "min_output"

CONFIG_SCHEMA = stepper.STEPPER_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(FeedbackFan),
        cv.Required(CONF_SENSOR_ID): cv.use_id(sensor.Sensor),
        cv.Required(CONF_FAN_ID): cv.use_id(hbridge),
        cv.Optional(CONF_INITIAL_OUTPUT): cv.int_range(
            min=1
        ),  # fix to grab speedcount
        cv.Optional(CONF_MIN_OUTPUT, default=0): cv.int_range(min=0),
    }
).extend(cv.COMPONENT_SCHEMA)


@automation.register_action(
    "stepper.feedback_fan.set_initial_output",
    SetInitialOutputAction,
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.use_id(FeedbackFan),
            cv.Required(CONF_INITIAL_OUTPUT): cv.templatable(cv.int_),
        }
    ),
)
async def feedback_fan_set_initial_output_to_code(
    config, action_id, template_arg, args
):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    template_ = await cg.templatable(config[CONF_INITIAL_OUTPUT], args, cg.int32)
    cg.add(paren.set_initial_output(template_))
    return var


# @automation.register_action(
#     "stepper.feedback_fan.set_home",
#     SetInitialOutputAction,
#     cv.Schema(
#         {
#             cv.Required(CONF_ID): cv.use_id(FeedbackFan),
#             cv.Required(CONF_INITIAL_OUTPUT): cv.templatable(cv.int_),
#         }
#     ),
# )


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await stepper.register_stepper(var, config)

    hbridge_fan_id = await cg.get_variable(config[CONF_FAN_ID])
    cg.add(var.set_hbridge_fan(hbridge_fan_id))
    feedback_sensor_id = await cg.get_variable(config[CONF_SENSOR_ID])
    cg.add(var.set_sensor(feedback_sensor_id))

    if CONF_INITIAL_OUTPUT in config:
        cg.add(var.set_initial_output(config[CONF_INITIAL_OUTPUT]))
    if CONF_MIN_OUTPUT in config:
        cg.add(var.set_min_output(config[CONF_MIN_OUTPUT]))
