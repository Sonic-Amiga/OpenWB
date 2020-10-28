OpenWB
======

Unofficial opensource firmware for WirenBoard (https://wirenboard.com/en/) ModBus hardware

## Motivation

1. Unfortunately WirenBoard company does not support older devices, lacking important features, like ability to change port speed. This circumstance forces to decomission otherwise fully working hardware.
2. Striving for knowledge and experiments. I can, so why not? :)

## Support

At the moment only old WB-MR2 (not mini) board is supported. I don't know Mini hardware, perhaps it's compatible

## Installation
* Build the project using CubeIDE
* Connect the ST-Link programmer to your board according to the supplied schematic (see Docs folder)
* Run a programmer software and connect to the board
* On previously unmodified board the controller will be read-protected. Deactivate the fuse. **WARNING** This action completely erases any original firmware from the microcontroller. It is **IRREVERSIBLE**
* Flash your new firmware

