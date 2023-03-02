# Enable this extension if you need a xxd tool during the build AmLogic u-boot.

function add_host_dependencies__add_arm64_c_plus_plus_compiler() {
	display_alert "Adding xxd tool to host dependencies" "xxd" "debug"
	export EXTRA_BUILD_DEPS="${EXTRA_BUILD_DEPS} xxd" # @TODO: convert to array later
}
