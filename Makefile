# ===========================
# Project & toolchain
# ===========================
BUILDDIR      := Tools
TARGET_NAME   := os_test
TARGET        := $(BUILDDIR)/$(TARGET_NAME)

CROSS         ?= arm-none-eabi-
CC            := $(CROSS)gcc
AS            := $(CROSS)gcc
OBJCOPY       := $(CROSS)objcopy
OBJDUMP       := $(CROSS)objdump
SIZE          := $(CROSS)size

# ===========================
# MCU / CMSIS / SPL
# ===========================
CPUFLAGS      := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
DEFINES       := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

INC_DIRS := \
  app \
  app/tasks \
  app/hooks \
  bsw/communication/canif \
  bsw/communication/pdur \
  bsw/communication/com \
  bsw/ecua/iohwab/inc \
  bsw/services/ecum \
  bsw/services/os/arch/cortexm3_stm32f1 \
  bsw/services/os/inc \
  platform/common \
  platform/bsp/cmsis \
  platform/spl/inc \
  bsw/mcal/adc \
  bsw/mcal/can \
  bsw/mcal/dio \
  bsw/mcal/port \
  bsw/mcal/pwm \
  rte/core/inc \
  rte/core/swc_if \
  swc/Swc_BrakeAcq \
  swc/Swc_CmdComposer \
  swc/Swc_DriveModeMgr \
  swc/Swc_GearSelector \
  swc/Swc_SafetyManager \
  swc/Swc_PedalAcq  \
  cfg/communication \
  cfg/ecua          \
  cfg/mcal          

INCLUDES := $(addprefix -I, $(INC_DIRS))

# ===========================
# C/ASM/LD flags
# ===========================
CFLAGS_COMMON := -Og -g3 -Wall -Wextra -Wno-unused-parameter \
                 -ffreestanding -fno-builtin \
                 -ffunction-sections -fdata-sections \
                 -MMD -MP
CFLAGS        := $(CPUFLAGS) $(DEFINES) $(INCLUDES) $(CFLAGS_COMMON)

ASFLAGS       := $(CPUFLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp

LDSCRIPT      := platform/bsp/linker/stm32f103.ld
LDFLAGS       := -T$(LDSCRIPT) -nostartfiles -nostdlib -static \
                 -Wl,--gc-sections -Wl,-Map=$(TARGET).map
LDFLAGS      += -specs=nano.specs -specs=nosys.specs
LDLIBS        := -Wl,--start-group -lc -lm -lgcc -Wl,--end-group

# ===========================
# Sources
# ===========================
# LƯU Ý: không viết kiểu "thư mục \n file.c". Hãy chỉ rõ đường dẫn đầy đủ
# hoặc dùng wildcard/addprefix.
SRCS_C := \
  app/main.c \
  $(wildcard app/tasks/*.c) \
  $(wildcard bsw/communication/canif/*.c) \
  $(wildcard bsw/communication/pdur/*.c) \
  $(wildcard bsw/communication/com/*.c) \
  $(wildcard bsw/ecua/iohwab/src/*.c) \
  $(wildcard bsw/mcal/adc/*.c)\
  $(wildcard bsw/mcal/can/*.c)\
  $(wildcard bsw/mcal/dio/*.c)\
  $(wildcard bsw/mcal/port/*.c)\
  $(wildcard bsw/mcal/PWM/*.c)\
  $(wildcard bsw/services/os/arch/cortexm3_stm32f1/*.c) \
  $(wildcard bsw/services/os/src/*.c) \
  $(wildcard bsw/services/ecum/*.c) \
  $(wildcard cfg/mcal/*.c)\
  $(wildcard cfg/ecua/*.c)\
  $(wildcard cfg/communication/*.c) \
  debug/semihost.c \
  debug/syscalls_min.c \
  $(wildcard platform/spl/src/*.c) \
  $(wildcard swc/*/*.c)\
  $(wildcard rte/core/src/*.c)

SRCS_S := \
  platform/bsp/startup_stm32f10x_md.s \
  bsw/services/os/arch/cortexm3_stm32f1/Os_Arch_Asm.s

# ===========================
# Objects / Deps
# ===========================
OBJS_C := $(patsubst %.c,$(BUILDDIR)/%.o,$(SRCS_C))
OBJS_S := $(patsubst %.s,$(BUILDDIR)/%.o,$(filter %.s,$(SRCS_S))) \
          $(patsubst %.S,$(BUILDDIR)/%.o,$(filter %.S,$(SRCS_S)))
OBJS   := $(OBJS_C) $(OBJS_S)
DEPS   := $(OBJS_C:.o=.d)
# Ensure build dir exists
$(shell mkdir -p $(BUILDDIR))
# ===========================
# Default goal
# ===========================
.PHONY: all
all: $(TARGET).bin size

# ===========================
# Compile rules
# ===========================
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(CPUFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# ===========================
# Link
# ===========================
$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@mkdir -p $(dir $@)
	$(CC) $(CPUFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# ===========================
# BIN/HEX/SIZE/Listing
# ===========================
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

.PHONY: size
size: $(TARGET).elf
	$(SIZE) --format=berkeley $<

.PHONY: list
list: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).list


# ===============================
# Nạp firmware
# ===============================
.PHONY: flash
flash: $(TARGET).elf
	@test -f $< || { echo "ELF not found: $<"; exit 2; }
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "adapter speed 2000" \
		-c "reset_config none separate" \
		-c "init; halt" \
		-c "stm32f1x options_read 0" \
		-c "program $< verify" \
		-c "reset run; shutdown"

# ===============================
# Clean
# ===============================
.PHONY: clean
clean:
	rm -rf $(BUILDDIR) $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).map $(TARGET).list

# Auto deps
-include $(DEPS)
