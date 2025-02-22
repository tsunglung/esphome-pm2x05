import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ADDRESS,
    CONF_ID,
    CONF_PM_1_0,
    CONF_PM_2_5,
    CONF_TYPE,
    CONF_UPDATE_INTERVAL,
    DEVICE_CLASS_PM10,
    DEVICE_CLASS_PM25,
    ICON_CHEMICAL_WEAPON,
    STATE_CLASS_MEASUREMENT,
    UNIT_MICROGRAMS_PER_CUBIC_METER
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor"]

pm2x05_ns = cg.esphome_ns.namespace("pm2x05")
PM2X05Component = pm2x05_ns.class_(
    "PM2X05Component", cg.Component, i2c.I2CDevice
)
PM2X05Sensor = pm2x05_ns.class_("PM2X05Sensor", sensor.Sensor)

TYPE_PM2005 = "PM2005"
TYPE_PM2105 = "PM2105"

PM2X05Type = pm2x05_ns.enum("PM2X05Type")

PM2X05_TYPES = {
    TYPE_PM2005: PM2X05Type.PM2X05_TYPE_2005,
    TYPE_PM2105: PM2X05Type.PM2X05_TYPE_2105
}

SENSORS_TO_TYPE = {
    CONF_PM_1_0: [TYPE_PM2005, TYPE_PM2105],
    CONF_PM_2_5: [TYPE_PM2005, TYPE_PM2105],
}

def validate_pm2x05_sensors(value):
    for key, types in SENSORS_TO_TYPE.items():
        if key in value and value[CONF_TYPE] not in types:
            raise cv.Invalid(f"{value[CONF_TYPE]} does not have {key} sensor!")
    return value


def validate_update_interval(value):
    value = cv.positive_time_period_milliseconds(value)
    if value == cv.time_period("0s"):
        return value
    if value < cv.time_period("30s"):
        raise cv.Invalid(
            "Update interval must be greater than or equal to 30 seconds if set."
        )
    return value


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PM2X05Component),
            cv.Required(CONF_TYPE): cv.enum(PM2X05_TYPES, upper=True),
            cv.Required(CONF_PM_1_0): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_PM10,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Required(CONF_PM_2_5): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_PM25,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ADDRESS): cv.i2c_address,
            cv.Optional(CONF_UPDATE_INTERVAL, default="0s"): validate_update_interval,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x28))
)

def final_validate(config):
    schema = i2c.final_validate_device_schema(
        "pm2x05", min_frequency="10khz"
    )
    schema(config)


FINAL_VALIDATE_SCHEMA = final_validate


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_type(config[CONF_TYPE]))

    if CONF_PM_1_0 in config:
        sens = await sensor.new_sensor(config[CONF_PM_1_0])
        cg.add(var.set_pm_1_0_sensor(sens))

    if CONF_PM_2_5 in config:
        sens = await sensor.new_sensor(config[CONF_PM_2_5])
        cg.add(var.set_pm_2_5_sensor(sens))

    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
