
# ==========================================
# AltROM Build System
# Toolchain: Clang / LLVM
# Target: ARMv5TE (Thumb)
# ==========================================

# Program name / main source file
NAME ?= altrom

# Directories
OBJDIR = obj/$(NAME)

# Source files
SRCS = start $(NAME)
OBJS = $(SRCS:%=$(OBJDIR)/%.o)

# Output files
ELF = $(OBJDIR)/$(NAME).elf
BIN = $(NAME).bin
ASM = $(NAME).asm

LDSCRIPT ?= simple.ld
LOAD_ADDR = 0x70000000

# Toolchain
CC = clang --target=arm-none-eabi
OBJCOPY = llvm-objcopy
OBJDUMP = llvm-objdump
SIZE = llvm-size

# ==========================================
# Compiler flags
# ==========================================

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

.PHONY: all clean rebuild size disasm run

all: $(BIN)

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Include dependency files
-include $(OBJS:.o=.d)

# ==========================================
# Compile C
# ==========================================

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) $< -c -o $@

# ==========================================
# Compile assembly
# ==========================================

$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(CC) $(ASFLAGS) $< -c -o $@

$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(CC) $(ASFLAGS) $< -c -o $@

# ==========================================
# Link ELF
# ==========================================

$(ELF): $(OBJS) $(LDSCRIPT)
	$(CC) $(LFLAGS) $(OBJS) -o $@

# ==========================================
# Generate binary
# ==========================================

$(BIN): $(ELF)
	$(OBJCOPY) -O binary -R .bss $< $@

# ==========================================
# Clean
# ==========================================

clean:
	$(RM) -r obj
	$(RM) *.bin *.asm

# ==========================================
# Rebuild
# ==========================================

rebuild:
	$(MAKE) NAME=$(NAME) clean
	$(MAKE) NAME=$(NAME) all

# ==========================================
# Display binary size
# ==========================================

size: $(ELF)
	$(SIZE) $<

# ==========================================
# Disassembly
# ==========================================

disasm: $(ELF)
	$(OBJDUMP) -d $< > $(ASM)

# ==========================================
# Run on device
# ==========================================

run: $(BIN)
	sudo ../mtk_dump connect simple_da payload.bin 0x70008000
	sudo ../mtk_dump connect simple_da $(BIN) $(LOAD_ADDR)

