LDFLAGS ?= -lftdi -lusb

PACKAGES = ftdi-devices-list ftdi-eeprom-config

all: ${PACKAGES}

ftdi-devices-list: devices_list.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

ftdi-eeprom-config: eeprom_config.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

install:
	cp $(PACKAGES) /usr/local/bin/
clean:
	rm -f $(PACKAGES)
