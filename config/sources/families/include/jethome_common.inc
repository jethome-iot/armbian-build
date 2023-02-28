
BOOTENV_FILE='meson.txt'
LINUXFAMILY='jethome64'
ARCH='arm64'
SERIALCON=ttyAML0
SRC_LOADADDR='LOADADDR=0x1080000'
UBOOT_TARGET_MAP="u-boot-dtb.img;;u-boot.bin.sd.bin:u-boot.bin u-boot.bin:u-boot.nosd.bin u-boot-dtb.img"
BOOTSCRIPT="boot-jethub.cmd:boot.cmd"
BOOTBRANCH="${BOOTBRANCH_BOARD:-"tag:v2022.07"}"
BOOTPATCHDIR="${BOOTPATCHDIR:-"v2022.07"}"
OVERLAY_PREFIX='jethub'

# this family does not need it
ATF_COMPILE="no"

# Set CPUMIN et al if not already set in the board config.
CPUMIN=${CPUMIN:-250000}
CPUMAX=${CPUMAX:-1536000}
GOVERNOR=${GOVERNOR:-ondemand}

case $BRANCH in

	current)
		export KERNEL_MAJOR_MINOR="6.1" # Major and minor versions of this kernel. For mainline caching.
		KERNELBRANCH='branch:linux-6.1.y'
		KERNELPATCHDIR='jethome64-current'
		;;

	edge)
		export KERNEL_MAJOR_MINOR="6.2" # Major and minor versions of this kernel. For mainline caching.
		KERNELBRANCH='branch:linux-6.2.y'     # @TODO: soon-to-be 'branch:linux-6.2.y' when it is released
		KERNELPATCHDIR='jethome64-edge'
		;;

esac

# this helper function includes postprocess for p212 and its variants.
# $1 PATH for uboot blob repo
# $2 dir name in uboot blob repo
uboot_gxl_postprocess() {
	run_host_command_logged mv -v u-boot.bin bl33.bin

	run_host_command_logged $1/blx_fix.sh $1/$2/bl30.bin \
		$1/$2/zero_tmp \
		$1/$2/bl30_zero.bin \
		$1/$2/bl301.bin \
		$1/$2/bl301_zero.bin \
		$1/$2/bl30_new.bin bl30

	run_host_command_logged python2 $1/acs_tool.pyc $1/$2/bl2.bin \
		$1/$2/bl2_acs.bin \
		$1/$2/acs.bin 0

	run_host_command_logged $1/blx_fix.sh $1/$2/bl2_acs.bin \
		$1/$2/zero_tmp \
		$1/$2/bl2_zero.bin \
		$1/$2/bl21.bin \
		$1/$2/bl21_zero.bin \
		$1/$2/bl2_new.bin bl2

	run_host_x86_binary_logged $1/$2/aml_encrypt_gxl --bl3enc --input $1/$2/bl30_new.bin
	run_host_x86_binary_logged $1/$2/aml_encrypt_gxl --bl3enc --input $1/$2/bl31.img
	run_host_x86_binary_logged $1/$2/aml_encrypt_gxl --bl3enc --input bl33.bin

	run_host_x86_binary_logged $1/$2/aml_encrypt_gxl --bl2sig --input $1/$2/bl2_new.bin \
		--output bl2.n.bin.sig

	run_host_x86_binary_logged $1/$2/aml_encrypt_gxl --bootmk \
		--output u-boot.bin \
		--bl2 bl2.n.bin.sig \
		--bl30 $1/$2/bl30_new.bin.enc \
		--bl31 $1/$2/bl31.img.enc \
		--bl33 bl33.bin.enc
}

# this helper function includes postprocess for s400 and its variants.
# $1 PATH for uboot blob repo
# $2 dir name in uboot blob repo
uboot_axg_postprocess_ng() {
	run_host_command_logged mv -v u-boot.bin bl33.bin

	run_host_command_logged $1/blx_fix.sh $1/$2/bl30.bin \
		$1/$2/zero_tmp \
		$1/$2/bl30_zero.bin \
		$1/$2/bl301.bin \
		$1/$2/bl301_zero.bin \
		$1/$2/bl30_new.bin bl30

	run_host_command_logged python3 $1/acs_tool.py $1/$2/bl2.bin \
		$1/$2/bl2_acs.bin \
		$1/$2/acs.bin 0

	run_host_command_logged $1/blx_fix.sh $1/$2/bl2_acs.bin \
		$1/$2/zero_tmp \
		$1/$2/bl2_zero.bin \
		$1/$2/bl21.bin \
		$1/$2/bl21_zero.bin \
		$1/$2/bl2_new.bin bl2

	run_host_x86_binary_logged $1/$2/aml_encrypt_axg --bl3sig --input $1/$2/bl30_new.bin \
		--output bl30_new.bin.enc \
		--level v3 --type bl30

	run_host_x86_binary_logged $1/$2/aml_encrypt_axg --bl3sig --input $1/$2/bl31.img \
		--output bl31.img.enc \
		--level v3 --type bl31

	run_host_x86_binary_logged $1/$2/aml_encrypt_axg --bl3sig --input bl33.bin --compress lz4 \
		--output bl33.bin.enc \
		--level v3 --type bl33

	run_host_x86_binary_logged $1/$2/aml_encrypt_axg --bl2sig --input $1/$2/bl2_new.bin \
		--output bl2.n.bin.sig

	run_host_x86_binary_logged $1/$2/aml_encrypt_axg --bootmk \
		--output u-boot.bin \
		--bl2 bl2.n.bin.sig \
		--bl30 bl30_new.bin.enc \
		--bl31 bl31.img.enc \
		--bl33 bl33.bin.enc --level v3
}

# this helper function includes postprocess for u200 and its variants.
# $1 PATH for uboot blob repo
# $2 family g12a or g12b
uboot_g12_postprocess() {
	run_host_command_logged mv -v u-boot.bin bl33.bin

	run_host_command_logged $1/blx_fix.sh $1/bl30.bin \
		zero_tmp \
		bl30_zero.bin \
		$1/bl301.bin \
		bl301_zero.bin \
		bl30_new.bin bl30

	run_host_command_logged $1/blx_fix.sh $1/bl2.bin \
		zero_tmp \
		bl2_zero.bin \
		$1/acs.bin \
		bl21_zero.bin \
		bl2_new.bin bl2

	run_host_x86_binary_logged $1/aml_encrypt_$2 --bl30sig \
		--input bl30_new.bin \
		--output bl30_new.bin.g12.enc \
		--level v3
	run_host_x86_binary_logged $1/aml_encrypt_$2 --bl3sig \
		--input bl30_new.bin.g12.enc \
		--output bl30_new.bin.enc \
		--level v3 --type bl30
	run_host_x86_binary_logged $1/aml_encrypt_$2 --bl3sig \
		--input $1/bl31.img \
		--output bl31.img.enc \
		--level v3 --type bl31
	run_host_x86_binary_logged $1/aml_encrypt_$2 --bl3sig \
		--input bl33.bin \
		--compress lz4 \
		--output bl33.bin.enc \
		--level v3 --type bl33
	run_host_x86_binary_logged $1/aml_encrypt_$2 --bl2sig \
		--input bl2_new.bin \
		--output bl2.n.bin.sig
	if [ -e $1/lpddr3_1d.fw ]; then
		run_host_x86_binary_logged $1/aml_encrypt_$2 --bootmk --output u-boot.bin \
			--bl2 bl2.n.bin.sig \
			--bl30 bl30_new.bin.enc \
			--bl31 bl31.img.enc \
			--bl33 bl33.bin.enc \
			--ddrfw1 $1/ddr4_1d.fw \
			--ddrfw2 $1/ddr4_2d.fw \
			--ddrfw3 $1/ddr3_1d.fw \
			--ddrfw4 $1/piei.fw \
			--ddrfw5 $1/lpddr4_1d.fw \
			--ddrfw6 $1/lpddr4_2d.fw \
			--ddrfw7 $1/diag_lpddr4.fw \
			--ddrfw8 $1/aml_ddr.fw \
			--ddrfw9 $1/lpddr3_1d.fw \
			--level v3
	else
		run_host_x86_binary_logged $1/aml_encrypt_$2 --bootmk --output u-boot.bin \
			--bl2 bl2.n.bin.sig \
			--bl30 bl30_new.bin.enc \
			--bl31 bl31.img.enc \
			--bl33 bl33.bin.enc \
			--ddrfw1 $1/ddr4_1d.fw \
			--ddrfw2 $1/ddr4_2d.fw \
			--ddrfw3 $1/ddr3_1d.fw \
			--ddrfw4 $1/piei.fw \
			--ddrfw5 $1/lpddr4_1d.fw \
			--ddrfw6 $1/lpddr4_2d.fw \
			--ddrfw7 $1/diag_lpddr4.fw \
			--ddrfw8 $1/aml_ddr.fw \
			--level v3
	fi

}

# this helper function includes postprocess for meson gxl and gxm.
# $1 PATH for uboot blob repo
uboot_gxl_postprocess_ng() {
	run_host_command_logged mv -v u-boot.bin bl33.bin

	run_host_command_logged $1/blx_fix.sh $1/bl30.bin \
		$1/zero_tmp \
		$1/bl30_zero.bin \
		$1/bl301.bin \
		$1/bl301_zero.bin \
		$1/bl30_new.bin bl30

	run_host_command_logged python3 $1/acs_tool.py $1/bl2.bin $1/bl2_acs.bin $1/acs.bin 0

	run_host_command_logged $1/blx_fix.sh $1/bl2_acs.bin \
		$1/zero_tmp \
		$1/bl2_zero.bin \
		$1/bl21.bin \
		$1/bl21_zero.bin \
		$1/bl2_new.bin bl2

	run_host_command_logged $1/aml_encrypt_gxl --bl3enc --input $1/bl30_new.bin
	run_host_command_logged $1/aml_encrypt_gxl --bl3enc --input $1/bl31.img
	run_host_command_logged $1/aml_encrypt_gxl --bl3enc --input bl33.bin
	run_host_command_logged $1/aml_encrypt_gxl --bl2sig --input $1/bl2_new.bin \
		--output bl2.n.bin.sig

	run_host_command_logged $1/aml_encrypt_gxl --bootmk --output u-boot.bin \
		--bl2 bl2.n.bin.sig \
		--bl30 $1/bl30_new.bin.enc \
		--bl31 $1/bl31.img.enc \
		--bl33 bl33.bin.enc
}

write_uboot_platform() {
	dd if=$1/u-boot.bin of=$2 bs=1 count=442 conv=fsync 2>&1
	dd if=$1/u-boot.bin of=$2 bs=512 skip=1 seek=1 conv=fsync 2>&1
}

family_tweaks_bsp() {
	mkdir -p $destination/etc/udev/rules.d
	mkdir -p $destination/usr/local/bin
}

# this helper function includes postprocess for s4 family.
# $1 PATH for uboot blob repo
uboot_s4_postprocess() {
	# ENV
	DEV_ACS_BIN_SIZE=4096
	BL30_BIN_SIZE=65536
	BL33_BIN_SIZE=1572864
	BUILD_PATH="$1"

	run_host_command_logged mv -v u-boot.bin bl33.bin

	#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	# functio mk_uboot
	function mk_uboot () {
		output_images=$1
		input_payloads=$2
		postfix=$3
		storage_type_suffix=$4
		chipset_variant_suffix=$5

		device_fip="${input_payloads}/device-fip.bin${postfix}"
		bb1st="${input_payloads}/bb1st${storage_type_suffix}${chipset_variant_suffix}.bin${postfix}"
		bl2e="${input_payloads}/blob-bl2e${storage_type_suffix}${chipset_variant_suffix}.bin${postfix}"
		bl2x="${input_payloads}/blob-bl2x.bin${postfix}"

		if [ ! -f ${device_fip} ] || \
		[ ! -f ${bb1st} ] || \
		[ ! -f ${bl2e} ] || \
		[ ! -f ${bl2x} ]; then
			echo fip:${device_fip}
			echo bb1st:${bb1st}
			echo bl2e:${bl2e}
			echo bl2x:${bl2x}
			echo "Error: ${input_payloads}/ bootblob does not all exist... abort"
			ls -la ${input_payloads}/
			exit -1
		fi

		file_info_cfg="${output_images}/aml-payload.cfg"
		file_info_cfg_temp=${temp_cfg}.temp

		bootloader="${output_images}/u-boot.bin${storage_type_suffix}${postfix}"
		sdcard_image="${output_images}/u-boot.bin.sd.bin${postfix}"

		ddr_fip="${input_payloads}/ddr-fip.bin"

		#cat those together with 4K upper aligned for sdcard
		align_base=4096
		total_size=0
		for file in ${bb1st} ${bl2e} ${bl2x} ${ddr_fip} ${device_fip}; do
			size=`stat -c "%s" ${file}`
			upper=$[(size+align_base-1)/align_base*align_base]
			total_size=$[total_size+upper]
			echo ${file} ${size} ${upper}
		done

		echo ${total_size}
		rm -f ${bootloader}
		dd if=/dev/zero of=${bootloader} bs=${total_size} count=1 status=none

		sector=512
		seek=0
		seek_sector=0
		dateStamp=SC2-`date +%Y%m%d%H%M%S`

		echo @AMLBOOT > ${file_info_cfg_temp}
		dd if=${file_info_cfg_temp} of=${file_info_cfg} bs=1 count=8 conv=notrunc &> /dev/null
		nItemNum=5
		nSizeHDR=$[64+nItemNum*16]
		printf "01 %02x %02x %02x 00 00 00 00" $[(nItemNum)&0xFF] $[(nSizeHDR)&0xFF] $[((nSizeHDR)>>8)&0xFF] \
			| xxd -r -ps > ${file_info_cfg_temp}
		cat ${file_info_cfg_temp} >> ${file_info_cfg}

		echo ${dateStamp} > ${file_info_cfg_temp}
		dd if=${file_info_cfg_temp} of=${file_info_cfg} bs=1 count=16 oflag=append conv=notrunc &> /dev/null

		index=0
		arrPayload=("BBST" "BL2E" "BL2X" "DDRF" "DEVF");
		nPayloadOffset=0
		nPayloadSize=0
		for file in ${bb1st} ${bl2e} ${bl2x} ${ddr_fip} ${device_fip}; do
			size=`stat -c "%s" ${file}`
			size_sector=$[(size+align_base-1)/align_base*align_base]
			nPayloadSize=$[size_sector]
			size_sector=$[size_sector/sector]
			seek_sector=$[seek/sector+seek_sector]
			#nPayloadOffset=$[sector*(seek_sector+1)]
			nPayloadOffset=$[sector*(seek_sector)]
			echo ${file} ${seek_sector} ${size_sector}
			dd if=${file} of=${bootloader} bs=${sector} seek=${seek_sector} conv=notrunc status=none

			echo ${arrPayload[$index]} > ${file_info_cfg_temp}.x
			index=$((index+1))
			dd if=${file_info_cfg_temp}.x of=${file_info_cfg_temp} bs=1 count=4 &> /dev/null
			rm -f ${file_info_cfg_temp}.x
			printf "%02x %02x %02x %02x %02x %02x %02x %02x 00 00 00 00" $[(nPayloadOffset)&0xFF] $[((nPayloadOffset)>>8)&0xFF] $[((nPayloadOffset)>>16)&0xFF] $[((nPayloadOffset)>>24)&0xFF] \
			$[(nPayloadSize)&0xFF] $[((nPayloadSize)>>8)&0xFF] $[((nPayloadSize)>>16)&0xFF] $[((nPayloadSize)>>24)&0xFF] | xxd -r -ps >> ${file_info_cfg_temp}
			dd if=${file_info_cfg_temp} of=${file_info_cfg} oflag=append conv=notrunc &> /dev/null
			rm -f ${file_info_cfg_temp}
			seek=$[(size+align_base-1)/align_base*align_base]
		done

		run_host_x86_binary_logged openssl dgst -sha256 -binary ${file_info_cfg} > ${file_info_cfg}.sha256
		cat ${file_info_cfg} >> ${file_info_cfg}.sha256
		#cat ${file_info_cfg}.sha256 >> ${file_info_cfg}
		rm -f ${file_info_cfg}
		mv -f ${file_info_cfg}.sha256 ${file_info_cfg}

		dd if=${file_info_cfg} of=${bootloader} bs=512 seek=332 conv=notrunc status=none

		if [ ${storage_type_suffix} == ".sto" ]; then
			echo "Image SDCARD"
			total_size=$[total_size+512]
			rm -f ${sdcard_image}
			dd if=/dev/zero of=${sdcard_image} bs=${total_size} count=1 status=none
			dd if=${file_info_cfg}   of=${sdcard_image} conv=notrunc status=none
			dd if=${bootloader} of=${sdcard_image} bs=512 seek=1 conv=notrunc status=none

			mv ${bootloader} ${output_images}/u-boot.bin${postfix}
		fi

		rm -f ${file_info_cfg}
	}

	# end funcrion mk_uboot
	#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	#! copy device_acs.bin to dbinit-params.bin
	dev_acs_size=$(stat -c %s "$1/device_acs.bin")

	if [ ${dev_acs_size} -gt ${DEV_ACS_BIN_SIZE} ]; then
		echo "chip acs size exceed limit ${DEV_ACS_BIN_SIZE}, $dev_acs_size"
		exit -1
	fi
	dd if=/dev/zero of=dvinit-params.bin bs=${DEV_ACS_BIN_SIZE} count=1 &> /dev/null
	dd if="${BUILD_PATH}/device_acs.bin" of=dvinit-params.bin conv=notrunc &> /dev/null

	#fill bb1st.* from 9600 (0x2580) with 4096 (0x1000) bytes with dbinit-params.bin

	"${BUILD_PATH}/bin/add-dvinit-params.sh" ${BUILD_PATH}/bb1st.sto${CHIPSET_VARIANT_SUFFIX}.bin.signed ${BUILD_PATH}/dvinit-params.bin ${BUILD_PATH}/bb1st.sto${CHIPSET_VARIANT_SUFFIX}.bin.signed ${CUR_SOC}
	"${BUILD_PATH}/bin/add-dvinit-params.sh" ${BUILD_PATH}/bb1st.usb${CHIPSET_VARIANT_SUFFIX}.bin.signed ${BUILD_PATH}/dvinit-params.bin ${BUILD_PATH}/bb1st.usb${CHIPSET_VARIANT_SUFFIX}.bin.signed ${CUR_SOC}

	#Check bl30.bin < 64KB
	# fill bl30-payload.bin with bl30.bin + zero bytes
	dd if=/dev/zero of=bl30-payload.bin bs=${BL30_BIN_SIZE} count=1 &> /dev/null
	dd if=${BUILD_PATH}/bl30.bin of=bl30-payload.bin conv=notrunc &> /dev/null


	#if we use compression for u-boot (bl33)
	if [ "y" == "y" ]; then
		mv -f bl33.bin  bl33.bin.org
		# echo compress bl33

		run_host_x86_binary_logged "$1/aml_encrypt_s4" --bl3sig  --input bl33.bin.org --output bl33.bin.org.lz4 --compress lz4 \
			--level v3 --type bl33

		#get LZ4 format bl33 image from bl33.bin.enc with offset 0x720
		dd if=bl33.bin.org.lz4 of=bl33.bin bs=1 skip=1824 >& /dev/null
	fi

	# max size of bl33 is 1572864
	if [ ! -f ${BUILD_PATH}/bl33.bin ]; then
		echo "Error: ${BUILD_PATH}/bl33.bin does not exist... abort"
		exit -1
	fi
	blx_size=`stat -c %s ${BUILD_PATH}/bl33.bin`
	if [ $blx_size -gt ${BL33_BIN_SIZE} ]; then
		echo "Error: bl33 size exceed limit ${BL33_BIN_SIZE}"
		exit -1
	fi

	dd if=/dev/zero of=bl33-payload.bin bs=${BL33_BIN_SIZE} count=1 &> /dev/null
	dd if=bl33.bin of=bl33-payload.bin conv=notrunc &> /dev/null

	# check blob-bl40.bin.signed
	# check device-fip-header.bin

	#create device-fip.bin.signed
	run_host_x86_binary_logged "$1/binary-tool/acpu-imagetool" create-device-fip \
		--infile-template-chipset-fip-header="$1/device-fip-header.bin" --infile-bl30-payload=bl30-payload.bin \
		--infile-bl33-payload=bl33-payload.bin --infile-blob-bl40="$1/blob-bl40.bin.signed" \
		--infile-blob-bl31="$1/blob-bl31.bin.signed" --infile-blob-bl32="$1/blob-bl32.bin.signed" \
		--outfile-device-fip=device-fip.bin.signed

	#fake ddr fip 256KB
	echo "==== use empty ddr-fip ===="
	dd if=/dev/zero of=ddr-fip.bin bs=1024 count=256 status=none

	#create u-boot sdcard
	mk_uboot fip/_tmp fip/_tmp .signed .sto

	#create u-boot usb
	mk_uboot fip/_tmp fip/_tmp .signed .usb

}


# This is an extension method, put directly in meson64_common. A "built-in" extension if you will.
# If used in more than one place, it could be moved to an extension: enable_extension "amlogic-fip-tools"
function fetch_sources_tools__amlogic_fip() {
	fetch_from_repo "https://github.com/jethome-iot/amlogic-boot-fip" "amlogic-boot-fip" "branch:jethub_j82"
}
