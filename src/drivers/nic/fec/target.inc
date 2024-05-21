DRIVER   := fec_nic_drv
TARGET   := $(DRIVER)
LIBS     := base imx_linux_generated imx_lx_emul jitterentropy

INC_DIR  := $(PRG_DIR)/../..
INC_DIR  += $(PRG_DIR)/include/lx_emul/shadow

SRC_CC   += main.cc
SRC_CC   += lx_emul/random.cc

SRC_C    += dummies.c
SRC_C    += lx_emul.c
SRC_C    += lx_emul/shadow/drivers/base/power/runtime.c
SRC_C    += lx_emul/shadow/drivers/char/random.c
SRC_C    += lx_user.c
SRC_C    += $(notdir $(wildcard $(PRG_DIR)/generated_dummies.c))

CC_OPT_drivers/base/regmap/regmap   += -I$(LX_SRC_DIR)/drivers/base/regmap

#CC_OPT_dummies  = -DKBUILD_MODFILE='"dummies"'
#CC_OPT_dummies += -DKBUILD_BASENAME='"dummies"'
#CC_OPT_dummies += -DKBUILD_MODNAME='"dummies"'
#
#CC_OPT_generated_dummies  = -DKBUILD_MODFILE='"dummies"'
#CC_OPT_generated_dummies += -DKBUILD_BASENAME='"dummies"'
#CC_OPT_generated_dummies += -DKBUILD_MODNAME='"dummies"'

# for including 'net-sysfs.h'
CC_OPT_dummies += -I$(LX_SRC_DIR)/net/core


#
# Genode C-API backends
#

SRC_CC  += genode_c_api/uplink.cc
GENODE_C_API_SRC_DIR := $(call select_from_repositories,src/lib/genode_c_api)
vpath % $(dir $(GENODE_C_API_SRC_DIR))

vpath %.c  $(PRG_DIR)/../..
vpath %.cc $(PRG_DIR)/../..
