# MQTT hardware switch

## Introduction

* Have an old Arduino Yún and a servo motor lying around?
* Need to automate a light switch etc. and don't want to deal with electrical
installation?

**We've got you covered!**

I used this setup to automate my old, not so smart bathroom fan without having
to fiddle around with the even older electrical installation of my apartment.
All you need to do is to fit the Arduino and the servo motor into a nice looking
box and upload this code. Before, make sure to copy the
[config template](./config.template.h) to `config.h` and fine-tune all values to
fit your needs.

*Et voilà!* Enjoy turning your device on/off using MQTT commands and
even integrate it in systems like **Home Assistant** with a minimum
effort. :tada:

![A fancy teaser](./doc/action.gif)

> :clapper: Take a look at the [doc](./doc) directory for more images and videos
of the device in action.

## Documentation

### MQTT messages

All topics are preceded by the base topic and a location tag, e.g.
`mqtt-switch/light-switch-bedroom`.

All message consist of a JSON formatted string and will be retained by default.

> Note the internal payload buffer limitation of 100 characters.

| Direction | Topic       | Description                     | Options/example                                       |
|-----------|-------------|---------------------------------|-------------------------------------------------------|
| In        | `/command`  | Request to operate the switch.  | `{"switch":"top"}` or `{"pos":42}` for testing        |
| Out       | `/state`    | Current state of the switch.    | `{"actual":"neutral","latest":"top"}`                 |
| Out       | `/avail`    | Availability indication (LWT).  | `{"state":"online","version":"Oct 30 2023 10:54:00"}` |

### Home Assistant sample configuration

```yml
mqtt:
  - switch:
      name: MQTT hardware switch
      availability:
        payload_available: online
        payload_not_available: offline
        topic: mqtt-switch/{location}/avail
        value_template: "{{ value_json.state }}"
      state_topic: mqtt-switch/{location}/state
      state_on: top
      state_off: bottom
      value_template: "{{ value_json.latest }}"
      command_topic: mqtt-switch/{location}/command
      payload_on: '{"switch":"top"}'
      payload_off: '{"switch":"bottom"}'
      json_attributes_topic: mqtt-switch/{location}/state
      optimistic: false
```

## Development

### A note on versioning

This sketch utilizes (of course!) [Semantic Versioning](https://semver.org/). A
macro constant `GIT_VERSION` is expected to be defined and will be printed out
at program start-up. As there is no generalized solution - *and that's a
shame!* - to fetch VCS version information at build time, providing this
information depends on your own individual workflow. Check out
[git-describe-arduino](https://github.com/fabianoriccardi/git-describe-arduino)
for a sophisticated workaround.

However, the sketch will also run without any version information and
automatically fall back to the compile date and time in that case.

### Linting

```sh
python -m venv venv
source ./venv/bin/activate
pip install --upgrade cpplint

cpplint --recursive --extensions=ino,c,cpp,h,hpp ./
`````
