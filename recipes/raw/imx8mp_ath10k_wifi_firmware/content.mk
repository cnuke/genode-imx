PORT_DIR := $(call port_dir,$(GENODE_DIR)/repos/dde_linux/ports/linux-firmware)

content: ucode_files LICENSE.wifi imx8mp_ath10k_wifi_firmware.tar


.PHONY: ucode_files
ucode_files:
	cp -R $(PORT_DIR)/firmware/ath10k .
	cp $(PORT_DIR)/firmware/regulatory.db .
	cp $(PORT_DIR)/firmware/regulatory.db.p7s .

LICENSE.wifi:
	for i in $(PORT_DIR)/firmware/ath10k/QCA9377/hw1.0/*.txt; do \
	echo "$${i##*/}:" >> $@; \
	  cat $$i >> $@; \
	  echo >> $@; \
	done

imx8mp_ath10k_wifi_firmware.tar: ucode_files LICENSE.wifi
	tar --mtime='2023-05-03 00:00Z' --remove-files \
	    --owner=0 --group=0 --numeric-owner --mode='go=' \
	    -cf $@ -C . ath10k/* && \
	rmdir ath10k
