#!/usr/bin/env bash
# Extension: jethub-burn
# Automatically converts Armbian .img into burn image after build

function run_after_build__999_jethub_burn() {

    if [[ "${BOARD}" != "jethubj100" ]]; then
        display_alert "jethub-burn" "Skipping burn conversion (board=${BOARD})" "info"
        return 0
    fi

    local IMG_PATH
    IMG_PATH=$(find "${SRC}/output/images" -type f -name "Armbian-*Jethubj100*.img" | sort | tail -n 1)

    if [[ -z "${IMG_PATH}" ]]; then
        display_alert "jethub-burn" "Burn conversion skipped: no image found" "wrn"
        return 0
    fi

    display_alert "Creating burn image" "$(basename "${IMG_PATH}")" "info"

    local IMAGES_DIR="${SRC}/output/images"
    local DEBS_DIR="${SRC}/output/debs"
    local TOOLS_DIR="${SRC}/extensions/jethub-burn-tools"

    local MAKE_BURN="${TOOLS_DIR}/make_burn.sh"
    local PACKER="${TOOLS_DIR}/tools/aml_image_v2_packer_new"
    local BINS_DIR="${TOOLS_DIR}/bins/j100"
    local IMAGE_CFG="${TOOLS_DIR}/bins/image.armbian.cfg"

    local DTS_DIR="${TOOLS_DIR}/dts"
    local DTS_NAME="meson-axg-jethome-jethub-j100.dts"

    local UBOOT_DEB
    UBOOT_DEB=$(ls -1 "${DEBS_DIR}"/linux-u-boot-jethubj100-current_*.deb 2>/dev/null | sort | tail -n1)
    [[ -f "${UBOOT_DEB}" ]] || exit_with_error "U-Boot deb not found in ${DEBS_DIR}"

    local TMP="${SRC}/.tmp/jethub-burn.$$"
    mkdir -p "${TMP}/deb-uboot"
    dpkg -x "${UBOOT_DEB}" "${TMP}/deb-uboot" || exit_with_error "dpkg -x U-Boot failed"

    local UBOOT_BIN
    UBOOT_BIN=$(find "${TMP}/deb-uboot/usr/lib" -type f -name "u-boot.nosd.bin" | head -n1)
    [[ -f "${UBOOT_BIN}" ]] || exit_with_error "u-boot.nosd.bin not found in U-Boot deb"

    [[ -x "${MAKE_BURN}" ]] || exit_with_error "make_burn.sh not executable: ${MAKE_BURN}"
    [[ -x "${PACKER}"    ]] || exit_with_error "packer not executable: ${PACKER}"
    [[ -r "${IMAGE_CFG}" ]] || exit_with_error "image config not found: ${IMAGE_CFG}"
    [[ -r "${BINS_DIR}/DDR.USB" && -r "${BINS_DIR}/UBOOT.USB" && -r "${BINS_DIR}/platform.conf" ]] \
        || exit_with_error "bins/j100 is incomplete (need DDR.USB, UBOOT.USB, platform.conf)"
    [[ -r "${DTS_DIR}/${DTS_NAME}" ]] || exit_with_error "DTS not found: ${DTS_DIR}/${DTS_NAME}"

    BINS_DIR="${BINS_DIR}" IMAGE_CFG="${IMAGE_CFG}" \
    PACKER_PATH="${PACKER}" UBOOT_BIN="${UBOOT_BIN}" \
    DTS_DIR="${DTS_DIR}" DTS_NAME="${DTS_NAME}" \
    bash "${MAKE_BURN}" "${IMG_PATH}" "j100"
    local rc=$?

    rm -rf "${TMP}"

    (( rc == 0 )) \
      && display_alert "jethub-burn" "Burn image created (see output/images)" "ok" \
      || exit_with_error "Burn image creation failed (rc=${rc})"
}
