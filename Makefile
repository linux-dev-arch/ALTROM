
# ==========================================
# AltROM Build System
# Toolchain: Clang / LLVM
# Target: ARMv5TE (Thumb)
# ==========================================

OBJDIR = obj
SRCS = start entry
OBJS = $(SRCS:%=$(OBJDIR)/%.o)

NAME = altrom
LDSCRIPT = simple.ld
LOAD_ADDR = 0x70008000
# Toolchain
CC = clang --target=arm-none-eabi
OBJCOPY = llvm-objcopy
OBJDUMP = llvm-objdump
SIZE = llvm-size

# Compiler flags
CFLAGS = -Oz -Wall \
         -ffreestanding \
         -fPIE \
         -march=armv5te \
         -mthumb \
         -fno-strict-aliasing \
         $(EXTRA_CFLAGS)

# Assembly flags
ASFLAGS = -march=armv5te -mthumb

# Linker flags
LFLAGS = -nostdlib \
         -fuse-ld=lld \
         -Wl,-T,$(LDSCRIPT) \
         -Wl,-z,notext

# Optional Link-Time Optimization
ifeq ($(LTO),1)
    CFLAGS += -flto
    LFLAGS += -flto
endif

# Optional sysroot
ifdef SYSROOT
    CFLAGS += --sysroot="$(SYSROOT)"
    LFLAGS += --sysroot="$(SYSROOT)"
endif

# ==========================================
# Targets
# ==========================================

.PHONY: all clean rebuild size disasm

all: $(NAME).bin

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Include dependency files
-include $(OBJS:.o=.d)

# Compile C
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) $< -c -o $@

# Compile assembly
$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(CC) $(ASFLAGS) $< -c -o $@

$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(CC) $(ASFLAGS) $< -c -o $@

# Link ELF
$(OBJDIR)/$(NAME).elf: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

# Generate binary
$(NAME).bin: $(OBJDIR)/$(NAME).elf
	$(OBJCOPY) -O binary -R .bss $< $@

# Clean
clean:
	$(RM) -r $(OBJDIR) $(NAME).bin $(NAME).asm

# Rebuild
rebuild: clean all

# Display binary size
size: $(OBJDIR)/$(NAME).elf
	$(SIZE) $<

# Generate disassembly
disasm: $(OBJDIR)/$(NAME).elf
	$(OBJDUMP) -d $< > $(NAME).asm

run: $(NAME).bin
	sudo ../mtk_dump connect simple_da $(NAME).bin $(LOAD_ADDR)
