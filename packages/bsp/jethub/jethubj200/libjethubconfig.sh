#!/bin/bash
# shellcheck disable=SC2034

GPIOCHIPNUMBER=0

GPIO_DIRECTION_OUTPUT=0
GPIO_DIRECTION_INPUT=1

GPIO_ACTIVE_LOW=0
GPIO_ACTIVE_HIGH=1

GPIOS=(
)


# Set LED states
LEDS=(
)


reset_zigbee() {
    echo "${0}: Reset Zigbee module ..."
    gpio_set ${GPIOCHIPNUMBER} 36 1 ${GPIO_ACTIVE_HIGH}
    gpio_set ${GPIOCHIPNUMBER} 41 1 ${GPIO_ACTIVE_HIGH}
    sleep 1
    gpio_set ${GPIOCHIPNUMBER} 41 0 ${GPIO_ACTIVE_HIGH}
}

config_1wire() {
    echo "${0}: Configure 1-Wire ..."
    if ! modprobe ds2482; then
        echo "${0}: *** Error: Failed to load DS2482 kernel module"
        exit 1
    fi

    sh -c "echo ds2482 0x18 > /sys/bus/i2c/devices/i2c-0/new_device" || true
}

ADDITIONALFUNC=""
