# T-Sticks

![main](https://github.com/markusritschel/icelab-tsticks/actions/workflows/main.yml/badge.svg)
[![License MIT license](https://img.shields.io/github/license/icelab-markusritschel/tsticks)](./LICENSE)

This repo contains the Arduino code which is needed to operate the temperature sticks (T-Sticks) in the ice laboratory of the Max-Planck-Insitute for Meteorology in Hamburg.

## Set up
Clone this repository on your machine
```bash
git clone https://github.com/markusritschel/icelab-tsticks
```
and then upload the code to your Arduino controller
```bash
arduino --port /dev/ttyACM0 --upload tsticks.ino
```

The scripts contain self-explanotary comments.


## Features
- [x] Iterate over multiple sensors that are attached to different pins on the Arduino
- [x] Sensors are automatically detected in the order of their physical appearance


## Maintainer
- [markusritschel](https://github.com/markusritschel)

## Contact & Issues
For any questions or issues, please contact me via git@markusritschel.de or open an [issue](https://github.com/markusritschel/icelab-tsticks/issues).


---
&copy; Markus Ritschel 2021
