#!/bin/bash
# shellcheck disable=SC2034

GPIOS=()
LEDS=()

UXM_SLOTS="1 2 3 4"

# MODE is latched when RESET is released: 0 - application, 1 - bootloader.
# Inverted compared to the BOOT line on J200. The modules power up with the
# lines floating and boot their application, but the expander keeps whatever it
# was last told to drive, so MODE is set explicitly rather than left over from a
# previous bootloader session.
UXM_MODE_APP=0
UXM_MODE_BOOTLOADER=1

# The UXM lines are addressed by the names from gpio-line-names, which the
# libgpiod v1 tools cannot do.
GPIOSET=${GPIOSET:-/usr/bin/gpioset}
GPIOINFO=${GPIOINFO:-/usr/bin/gpioinfo}
UXM_GPIOD_V2=""

check_uxm_gpiod() {
	if [[ -z "${UXM_GPIOD_V2}" ]]; then
		if ${GPIOSET} --help 2>&1 | grep -q -- "--by-name"; then
			UXM_GPIOD_V2="yes"
		else
			UXM_GPIOD_V2="no"
		fi
	fi
	if [[ "${UXM_GPIOD_V2}" != "yes" ]]; then
		echo "${0}: *** Error: UXM control needs the libgpiod v2 tools"
		return 1
	fi
	return 0
}

check_uxm_lines() {
	local slot="${1}"

	if ! ${GPIOINFO} "UXM${slot}_RESET" "UXM${slot}_MODE" > /dev/null 2>&1; then
		echo "${0}: *** Error: UXM${slot} GPIO lines not found"
		return 1
	fi
	return 0
}

# Leave MODE driven at the wanted level without keeping a process around:
# --toggle ends on the inverted value and returns immediately, so the opposite
# level goes in, and the expander holds its output latch after the release.
set_uxm_mode() {
	local slot="${1}"
	local level="${2}"

	${GPIOSET} --toggle 10ms,0 --by-name "UXM${slot}_MODE=$((1 - level))"
}

reset_uxm() {
	local slot="${1}"

	check_uxm_gpiod || return 1
	check_uxm_lines "${slot}" || return 1

	echo "${0}: Reset UXM${slot} module ..."
	set_uxm_mode "${slot}" "${UXM_MODE_APP}"
	${GPIOSET} --toggle 500ms,0 --by-name "UXM${slot}_RESET=1"
}

# Selecting the bootloader needs MODE driven across the release of RESET, which
# outlives a single gpioset call. Not used at boot.
reset_uxm_bootloader() {
	local slot="${1}"

	check_uxm_gpiod || return 1
	check_uxm_lines "${slot}" || return 1

	echo "${0}: Reset UXM${slot} module into its bootloader ..."
	set_uxm_mode "${slot}" "${UXM_MODE_BOOTLOADER}"
	${GPIOSET} --toggle 500ms,0 --by-name "UXM${slot}_RESET=1"
	sleep 1
	set_uxm_mode "${slot}" "${UXM_MODE_APP}"
}

reset_uxm_all() {
	local slot
	for slot in ${UXM_SLOTS}; do
		reset_uxm "${slot}" || true
	done
}

ADDITIONALFUNC="reset_uxm_all"
