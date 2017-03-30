# iot-lab-ftdi-utils
Tools to read/write iot-lab nodes ftdi eeprom.

## `ftdi-devices-list`

Lists FT2232 and FT4232 ftdi devices eeprom configuration

```
FTx232 devices lister by IoT-LAB
usage: ./ftdi-devices-list [-t ftdi_type] [-r]
  t: ftdi_type in [2232, 4232] default 2232
  r: reset ftdi devices
```

## `ftdi-eeprom-config`

Write FT2232 and FT4232 ftdi devices eeprom configuration for IoT-LAB Nodes

```
FTx232 EEPROM writer by IoT-LAB
Usage: ./ftdi-eeprom-config [-t ftdi_type] [-e current_name] [-s new_serial]
new_name
  t: ftdi_type in [2232, 4232] default 2232
  e: FTDI current name
  s: SerialNumber
```


### Support for libftdi1 ###

Added support for libftdi1 in source files.

To use it, LDFLAGS and CFLAGS should be adapted.
Set `-DLIBFTDI1=1` in CFLAGS and uptade LDFLAGS to use libftdi1
