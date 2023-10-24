# Arduino Yun MQTT switch

Why not automate a light switch mechanically using an Arduino Yún with a servo
motor attached?

## Home Assistant configuration

```yml
mqtt:
  - switch:
      name: Arduino Yun MQTT switch
      state_topic: yun-switch/state
      state_on: top
      state_off: bottom
      value_template: "{{ value_json.latest }}"
      command_topic: yun-switch/command
      payload_on: '{"state":"top"}'
      payload_off: '{"state":"bottom"}'
      json_attributes_topic: yun-switch/state
      optimistic: false
```
