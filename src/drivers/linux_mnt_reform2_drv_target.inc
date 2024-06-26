#
# Lx_emul + Lx_kit definitions
#

SRC_CC  += lx_emul/alloc.cc
SRC_CC  += lx_emul/clock.cc
SRC_CC  += lx_emul/debug.cc
SRC_CC  += lx_emul/init.cc
SRC_CC  += lx_emul/io_mem.cc
SRC_CC  += lx_emul/irq.cc
SRC_CC  += lx_emul/irq_flags.cc
SRC_CC  += lx_emul/log.cc
SRC_CC  += lx_emul/page_virt.cc
SRC_CC  += lx_emul/task.cc
SRC_CC  += lx_emul/time.cc

SRC_C   += common_dummies.c
SRC_C   += lx_emul/clocksource.c
SRC_C   += lx_emul/start.c
SRC_C   += lx_emul/shadow/drivers/base/power/common.c
SRC_C   += lx_emul/shadow/drivers/base/power/main.c
SRC_C   += lx_emul/shadow/drivers/base/power/runtime.c
SRC_C   += lx_emul/shadow/drivers/clk/clk.c
SRC_C   += lx_emul/shadow/drivers/clk/clkdev.c
SRC_C   += lx_emul/shadow/fs/exec.c
SRC_C   += lx_emul/shadow/kernel/cpu.c
SRC_C   += lx_emul/shadow/kernel/dma/mapping.c
SRC_C   += lx_emul/shadow/kernel/exit.c
SRC_C   += lx_emul/shadow/kernel/fork.c
SRC_C   += lx_emul/shadow/kernel/irq_work.c
SRC_C   += lx_emul/shadow/kernel/irq/spurious.c
SRC_C   += lx_emul/shadow/kernel/locking/spinlock.c
SRC_C   += lx_emul/shadow/kernel/pid.c
SRC_C   += lx_emul/shadow/kernel/printk/printk.c
SRC_C   += lx_emul/shadow/kernel/rcu/srcutree.c
SRC_C   += lx_emul/shadow/kernel/rcu/tree.c
SRC_C   += lx_emul/shadow/kernel/sched/cputime.c
SRC_C   += lx_emul/shadow/kernel/sched/core.c
SRC_C   += lx_emul/shadow/kernel/sched/fair.c
SRC_C   += lx_emul/shadow/kernel/sched/isolation.c
SRC_C   += lx_emul/shadow/kernel/sched/loadavg.c
SRC_C   += lx_emul/shadow/kernel/sched/sched.c
SRC_C   += lx_emul/shadow/kernel/smp.c
SRC_C   += lx_emul/shadow/kernel/stop_machine.c
SRC_C   += lx_emul/shadow/lib/cpumask.c
SRC_C   += lx_emul/shadow/lib/delay.c
SRC_C   += lx_emul/shadow/lib/devres.c
SRC_C   += lx_emul/shadow/lib/smp_processor_id.c
SRC_C   += lx_emul/shadow/mm/memblock.c
SRC_C   += lx_emul/shadow/mm/page_alloc.c
SRC_C   += lx_emul/shadow/mm/percpu.c
SRC_C   += lx_emul/shadow/mm/slab_common.c
SRC_C   += lx_emul/shadow/mm/slub.c
SRC_C   += lx_emul/shadow/mm/vmstat.c
SRC_C   += lx_emul/spec/arm/irqchip.c
SRC_C   += lx_emul/spec/arm/start.c
SRC_C   += lx_emul/virt_to_page.c

SRC_CC  += lx_kit/console.cc
SRC_CC  += lx_kit/device.cc
SRC_CC  += lx_kit/env.cc
SRC_CC  += lx_kit/init.cc
SRC_CC  += lx_kit/memory.cc
SRC_CC  += lx_kit/memory_dma.cc
SRC_CC  += lx_kit/scheduler.cc
SRC_CC  += lx_kit/task.cc
SRC_CC  += lx_kit/timeout.cc

DDE_LINUX_DIR := $(subst /src/include/lx_kit,,$(call select_from_repositories,src/include/lx_kit))

vpath % $(DDE_LINUX_DIR)/src/lib


#
# Linux kernel definitions
#

LX_CONTRIB_DIR  := $(call select_from_ports,mnt_reform2_linux)/linux

#
# Quirk needed as task->state has been renamed to task->__state in Linux 5.14
# and we are on >= 5.14 in lx_emul
#
CC_C_OPT += -Dstate=__state

CC_OPT_drivers/base/regmap/regmap  += -I$(LX_CONTRIB_DIR)/drivers/base/regmap
CC_OPT_lib/dynamic_debug           += -Wno-unused-variable
CC_OPT_lib/vsprintf                += -Wno-unused-variable

# avoid build errors because with CC_OLEVEL = -O0/-Og - not supported by Linux
override CC_OLEVEL := -O2

LX_SRC   = $(shell grep ".*\.c" $(PRG_DIR)/source.list)
SRC_S   += $(shell grep ".*\.S" $(PRG_DIR)/source.list)
SRC_C   += $(LX_SRC)
SRC_S   += $(LX_ASM:$(LX_CONTRIB_DIR)/%=%)

#
# Create symbol alias for jiffies, sharing the value of jiffies_64
#
LD_OPT += --defsym=jiffies=jiffies_64

vpath %.c $(LX_CONTRIB_DIR)
vpath %.S $(LX_CONTRIB_DIR)

# Define per-compilation-unit CC_OPT defines needed by MODULE* macros in Linux
define CC_OPT_LX_RULES =
CC_OPT_$(1) += -DKBUILD_MODFILE='"$(1)"' -DKBUILD_BASENAME='"$(notdir $(1))"' -DKBUILD_MODNAME='"$(notdir $(1))"'
endef

$(foreach file,$(LX_SRC),$(eval $(call CC_OPT_LX_RULES,$(file:%.c=%))))


#
# Generate driver-specific device-tree binary data
#
# The rules below use the tool/dts/extract tool to generate a device tree
# containing the driver parameters for a given board.
#
# The resulting dtb file is named <driver>-<board>.dtb
#

DTS_TOOL := $(BASE_DIR)/../../tool/dts/extract

CUSTOM_TARGET_DEPS += $(addprefix $(INSTALL_DIR)/$(TARGET)-,$(addsuffix .dtb,$(BOARDS)))

$(INSTALL_DIR)/%.dtb: %.dtb
	$(VERBOSE)cp -f $< $@

%.dtb: %.dts
	$(VERBOSE)dtc -q -Idts $< > $@

# dependencies of driver-specifc dts files from board's dts files
$(foreach B,$(BOARDS),$(eval $(TARGET)-$B.dts: $(LX_CONTRIB_DIR)/${DTS_PATH($B)}))

# normalize the interrupt controllers to one common name
$(TARGET)-%.dts:
	$(VERBOSE)$(CROSS_DEV_PREFIX)cpp -I$(LX_CONTRIB_DIR)/include \
	          -x assembler-with-cpp -MMD -P $(LX_CONTRIB_DIR)/${DTS_PATH($*)} |\
	          sed -s 's/intc\:/gic\:/' | \
	          sed -s 's/interrupt-parent = <\&intc>;/interrupt-parent = <\&gic>;/' | \
	          sed -s 's/interrupt-parent = <\&gpc>;/interrupt-parent = <\&gic>;/' | \
	          $(DTS_TOOL) ${DTS_EXTRACT($*)} - > $@


#
# The rule that follow is copied from
# 'repos/dde_linux/lib/import/import-lx_emul_common.inc'
# and needs to be kept in sync. When the MNT Reform Linux base is upgraded to
# the current DDE Linux version, this file will become obsolete.
#

#
# Generate initcall_table.c and create 'lx_emul_register_initcalls' which  calls
# global '__initptr_*' functions created by 'module_init'
#

define print_file_header
echo "/*"                                                          > $(2);
echo " * \\brief  Register initcalls from __initptr_*"            >> $(2);
echo " * \\author Automatically generated file - do no edit"      >> $(2);
echo " * \\date   $(1)"                                           >> $(2);
echo " */"                                                        >> $(2);
echo ""                                                           >> $(2);
echo ""                                                           >> $(2);
endef

define print_declaration
echo "extern void * $(1);" >> $(2);
endef

define print_function_start
echo ""                                      >> $(1)
echo "void lx_emul_register_initcalls(void)" >> $(1);
echo "{"                                     >> $(1);
echo "	typedef void (*func)(void);"         >> $(1);
endef

define print_call
echo "	((func)$(1))();" >> $(2);
endef

define print_function_end
echo "}" >> $(1);
endef


# 'module_init' calls should only be in C-sources
INITCALL_OBJECTS = $(addsuffix .o,$(basename $(SRC_C)))

# retrieve 'initptr_*' using nm from object files
INITCALLS = $(sort $(shell $(NM) --defined-only $(INITCALL_OBJECTS) |\
              grep "__initptr" |\
              awk '{print $$3}'))

OBJ_POSTPROC_SRC += initcall_table.c

initcall_table.c:
	$(MSG_CONFIG)$@
	@$(call print_file_header,$(shell date +"%F"),$@)
	@$(foreach sym,$(INITCALLS),$(call print_declaration,$(sym),$@))
	@$(call print_function_start,$@)
	@$(foreach sym,$(INITCALLS),$(call print_call,$(sym),$@))
	@$(call print_function_end,$@)
