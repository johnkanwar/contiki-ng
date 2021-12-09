#Call this one to build one example.
#Go into secure_zone and build.
#cd out and cd into non_secure_zone and build.
#Call a script that merge both hex files.
#Flash that hex file.

example_1 = 1
#secure_build = 1

ifeq ($(example_1),1)
    CFLAGS += -D SPM_EXAMPLE=1
    CFLAGS += -D TFM_EXAMPLE=0
else
    CFLAGS += -D SPM_EXAMPLE=0
    CFLAGS += -D TFM_EXAMPLE=1
endif



test:
	cd secure_zone && $(MAKE) clean \
	&& cd ../non_secure_zone $(MAKE) clean

rm:
	$(info	Removing builds) \
	cd secure_zone && sudo rm -R build \
	&& cd ../non_secure_zone && sudo rm -R build

secure:
	cd secure_zone && make TARGET=nrf BOARD=nrf5340/dk/application && sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

non-secure:
	cd non_secure_zone && make TARGET=nrf BOARD=nrf5340/dk/application && sudo make TARGET=nrf BOARD=nrf5340/dk/application example_ns.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

#Test remove the \ and && before the expressions.
tz:
	@echo "Building the secure build\n"
	cd secure_zone && make TARGET=nrf BOARD=nrf5340/dk/application \
	&& cp build/nrf/nrf5340/dk/application/example_s.hex ../example_s.hex
	cd non_secure_zone && make TARGET=nrf BOARD=nrf5340/dk/application \
	&& cp build/nrf/nrf5340/dk/application/example_ns.hex ../example_ns.hex \
	&& cd ../\
	&& cp example_s.hex merged_hex_file.hex \
	&& sed -i -e 's/:00000001FF//g' merged_hex_file.hex \
	&& echo "\n" >> merged_hex_file.hex \
	&& cat example_ns.hex >> merged_hex_file.hex
	sudo rm -R example_s.hex && sudo rm -R example_ns.hex
	cp merged_hex_file.hex secure_zone/build/nrf/nrf5340/dk/application/example_s.hex
	cd secure_zone/ \
	&& sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

run:
	cd secure_zone/ \
	&& sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

# How the script works.
# Go into secure and non-secure zone to build.
# Copy respecitve hex file and merge them.
# Go into secure_zone/Build/.. and replace hex with merged hex.
# Flash the hex file.
