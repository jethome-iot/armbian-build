#
# SPDX-License-Identifier: GPL-2.0
#
# This file is a part of the Armbian Build Framework
# https://github.com/armbian/build/
#
# Mainline kernel settings shared by Amlogic S7 (S905Y5) and S7D (S905X5M) boards.
# U-Boot, FIP and BSP stay in the board family (jethub.conf). Source this file
# from the family for every non-vendor branch.

declare -g LINUXFAMILY="meson-s7"
declare -g SERIALCON="ttyS0"

# The board config points at the vendor tree. Mainline branches use the
# default tree picked by mainline-kernel.conf.sh for KERNEL_MAJOR_MINOR.
unset KERNELSOURCE KERNELBRANCH

case "${BRANCH}" in
	edge)
		declare -g KERNEL_MAJOR_MINOR="7.3"
		;;
	*)
		exit_with_error "meson-s7: unsupported BRANCH='${BRANCH}' for BOARD='${BOARD}'"
		;;
esac

# S7 platform drivers that are modules or absent in the meson64 config.
# The config file starts as a copy of linux-meson64-edge.config.
function custom_kernel_config__meson_s7_soc_drivers() {
	display_alert "Enabling Amlogic S7/S7D SoC drivers" "${LINUXFAMILY}-${BRANCH}" "info"
	opts_y+=(
		CONFIG_PINCTRL_AMLOGIC_A4
		CONFIG_MESON_SM
		CONFIG_MESON_SECURE_PM_DOMAINS
		CONFIG_MESON_IRQ_GPIO
		CONFIG_RESET_MESON
		CONFIG_MESON_CLK_MEASURE
		CONFIG_MESON_GX_SOCINFO
		CONFIG_SERIAL_MESON
		CONFIG_SERIAL_MESON_CONSOLE
		CONFIG_PWM_MESON
		CONFIG_ARM_SCMI_PROTOCOL
		CONFIG_ARM_SCMI_TRANSPORT_SMC
		CONFIG_COMMON_CLK_SCMI
		CONFIG_ARM_SCMI_CPUFREQ
		CONFIG_COMMON_CLK_S7_PERIPHERALS
	)
	# meson_uart registers ttyS0 in 7.x; the 8250 placeholders steal the name
	# and the serial console gets no getty.
	opts_n+=(
		CONFIG_SERIAL_8250
	)
}
