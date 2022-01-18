example_1 = 1
#secure_build = 1

ifeq ($(example_1),1)
    CFLAGS += -D SPM_EXAMPLE=1
    CFLAGS += -D TFM_EXAMPLE=0
else
    CFLAGS += -D SPM_EXAMPLE=0
    CFLAGS += -D TFM_EXAMPLE=1
endif


rm:
	$(info	Removing builds) \
	cd secure_zone && sudo rm -R build \
	&& cd ../non_secure_zone && sudo rm -R build

secure:
	cd secure_zone && make TARGET=nrf BOARD=nrf5340/dk/application && sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

non-secure:
	cd non_secure_zone && make TARGET=nrf BOARD=nrf5340/dk/application && sudo make TARGET=nrf BOARD=nrf5340/dk/application example_ns.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

	
#mkdir and put the files there for better structure
#Preprocess linker scripts as they contain defines for memory addresses.
pre_process:
	@echo "~ ~Preprocessing linker scripts~ ~\n"
	cd mdk && arm-none-eabi-gcc -E -x c -Iinclude nrf5340_xxaa_application_ns_my_SPM.ld | grep -v '^#' > nrf5340_xxaa_application_ns_my_SPM_build.ld \
	&& arm-none-eabi-gcc -E -x c -Iinclude nrf5340_xxaa_application_SPM.ld | grep -v '^#' > nrf5340_xxaa_application_SPM_build.ld
	
# build secure and non-secure build, merge them
tz_build:
	$(MAKE) pre_process
	@echo "~ ~Building the secure and non-secure build~ ~\n"
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

# Lazy hardcoded flash to port 2
tz:
	$(MAKE) tz_build
	cd secure_zone/ \
	&& sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

window:
	cd secure_zone/ \
	&& make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2

run:
	cd secure_zone/ \
	&& sudo make TARGET=nrf BOARD=nrf5340/dk/application example_s.upload PORT=/dev/ttyACM2 && make TARGET=nrf BOARD=nrf5340/dk/application login PORT=/dev/ttyACM2
	

