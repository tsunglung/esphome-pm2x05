ESPHome PM2x05 Laster Particle Sensor Custom Component


example:

```
external_components:
  - source: github://tsunglung/esphome-pm2x05@master
    components: [ pm2x05 ]


i2c:
  - id: pm25_bus
    sda: 23
    scl: 22
    scan: true

sensor:
  - platform: pm2x05
    i2c_id: pm25_bus
    type: PM2005
    pm_2_5:
      name: "PM2.5 Sensor"
      id: pm25sensor
    pm_1_0:
      name: "PM1.0 Sensor"
      id: pm10sensor
    update_interval: 30s

```