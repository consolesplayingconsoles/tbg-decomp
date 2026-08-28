MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

ASMSH_FLAGS=-debug -cpu=sh4 -endian=little -sjis
BUILD_DIR=build
OUTPUT_DIR=$(BUILD_DIR)/output
SHA1_CHECKSUM=a6df9e0de39b2d11e9339aef915d20e35763ec81
SHELL := /bin/bash

# SERIAL_DEBUG=1 enables serial logging (default, matches shipped behavior);
# LOG_LEVEL (e.g. DEBUG, INFO, ...) overrides the default DEBUG_LEVEL threshold
# from src/serial_debug.h. Example: make SERIAL_DEBUG=1 LOG_LEVEL=DEBUG clean all
SERIAL_DEBUG ?= 1
LOG_LEVEL ?=
# DEBUG_MENU=1 boots straight into the debug menu instead of the title (and
# installs the Ninja print font it needs). Dev-only; not byte-matching.
DEBUG_MENU ?=
# GAME_LANG=en selects the (in-progress) half-width English text mode in
# 014f54_text.c instead of the default Japanese one. Not named LANG: that's
# a standard shell env var and would be inherited silently.
GAME_LANG ?= ja

SHC_DEFINES := __SHC__
ifeq ($(SERIAL_DEBUG),1)
SHC_DEFINES += SERIAL_DEBUG
endif
ifeq ($(DEBUG_MENU),1)
SHC_DEFINES += DEBUG_MENU
endif
ifeq ($(GAME_LANG),en)
SHC_DEFINES += GAME_LANG_EN
endif
ifneq ($(LOG_LEVEL),)
SHC_DEFINES += DEBUG_LEVEL=LOG_LEVEL_$(LOG_LEVEL)
endif

empty :=
space := $(empty) $(empty)
comma := ,
SHC_DEFINE_ARG = -define=$(subst $(space),$(comma),$(SHC_DEFINES))

SRCS = \
	src/010080_main.c \
	src/0100bc_sound.c \
	src/010e90.c \
	src/010fe8_heap.c \
	src/011120_asset_queues.c \
	src/012324_peripheral_support.c \
	src/012504_input.c \
	src/0129cc_pause.c \
	src/012f44_game.c \
	src/013ae8_route_load.c \
	src/014934.c \
	src/0149b0_sbinit.c \
	src/014a9c_tasks.c \
	src/014b8c_backup.c \
	src/014f54_text.c \
	src/015ab8_title.c \
	src/016108.c \
	src/01614c_debug_menu.c \
	src/016bf4_demo_input.c \
	src/016c58_prompt.c \
	src/016d2c_course_menu.c \
	src/018644_file_menu.c \
	src/0193c8_vm_menu.c \
	src/019e98_main_menu.c \
	src/01a148_option.c \
	src/01b19c_system_menu.c \
	src/01bb48_vm_game.c \
	src/01c980_profile_file.c \
	src/01d290_album.c \
	src/01d7fc_results.c \
	src/01e27c_practice_menu.c \
	src/asm/01f3c0.src \
	src/asm/01fa78.src \
	src/asm/020214.src \
	src/020528.c \
	src/asm/020594.src \
	src/0206f0_intersect.c \
	src/0207d4.c \
	src/asm/02081c.src \
	src/020914_ground_query.c \
	src/020b6c_ground_probe.c \
	src/02171c_tile_stream.c \
	src/asm/021b9c.src \
	src/0222dc_fadecmd.c \
	src/022464_fade.c \
	src/asm/022bdc.src \
	src/asm/023310.src \
	src/asm/023938.src \
	src/asm/02412c.src \
	src/asm/024280.src \
	src/asm/024b4c.src \
	src/asm/025870.src \
	src/asm/025b98.src \
	src/026710_traffic.c \
	src/02786c_vehicle_parts.c \
	src/asm/027958.src \
	src/028258_objects.c \
	src/02af78_event.c \
	src/asm/02b2f0.src \
	src/asm/02b464.src \
	src/02c884_bus_stop.c \
	src/asm/02d06c.src \
	src/asm/02d19c.src \
	src/asm/02d968.src \
	src/asm/02df3c.src \
	src/asm/02e2dc.src \
	src/asm/02e400.src \
	src/asm/02e51c.src \
	src/asm/02f0c8.src \
	src/asm/02f320.src \
	src/scif.c \
	src/serial_debug.c \
	src/asm/sectionD.src \
	src/asm/04ce10_slots.c \
	src/asm/sectionB.src \
	src/02fb50_sh4nlfzn_post_data.c \

C_SRCS = $(filter %.c,$(SRCS))

OBJS = $(patsubst src/%.c,$(OUTPUT_DIR)/src/%.obj,$(SRCS))
OBJS := $(patsubst src/asm/%.src,$(OUTPUT_DIR)/src/asm/%.obj,$(OBJS))
LINKER_OBJS = $(subst /,\\, $(OBJS))

all: create_dirs $(OUTPUT_DIR)/tbg.bin

create_dirs:
	@mkdir -p $(OUTPUT_DIR)/src/asm

# The defines are baked into every object, but make only compares timestamps --
# so flipping GAME_LANG/SERIAL_DEBUG/... would otherwise leave stale objects
# built with the old set. Rewrite the stamp only when the set actually changes.
DEFINES_STAMP := $(BUILD_DIR)/.defines
$(shell mkdir -p $(BUILD_DIR); \
        [ "$$(cat $(DEFINES_STAMP) 2>/dev/null)" = "$(SHC_DEFINES)" ] \
        || printf '%s' "$(SHC_DEFINES)" > $(DEFINES_STAMP))

$(OUTPUT_DIR)/src/asm/%.obj: src/asm/%.src $(DEFINES_STAMP)
	wibo "$(SHC_BIN)/asmsh.exe" "$(subst /,\\,$<)" -object="$(subst /,\\,$@)" $(ASMSH_FLAGS)

$(OUTPUT_DIR)/src/%.obj: src/%.c $(DEFINES_STAMP)
	wibo "$(SHC_BIN)/shc.exe" "$(subst /,\\,$<)" -object="$(subst /,\\,$@)" -sub=$(BUILD_DIR)/shc.sub $(SHC_DEFINE_ARG)
	wibo "$(SHC_BIN)/shc.exe" "$(subst /,\\,$<)" -code=asm -object="$(subst /,\\,$@).src" -sub=$(BUILD_DIR)/shc.sub

$(OUTPUT_DIR)/tbg.elf: $(OBJS) $(BUILD_DIR)/lnk.sub
	wibo "$(SHC_BIN)/lnk.exe" -sub=build\\lnk.sub

$(BUILD_DIR)/lnk.sub: $(BUILD_DIR)/lnk_template.sub Makefile
	sed "s|@DC_SDK@|$$(printf %q "$(KATANA_SDK_DIR)")|g" $(BUILD_DIR)/lnk_template.sub > $(BUILD_DIR)/lnk.sub
	sed -i 's|@INPUTS@|$(foreach obj,$(LINKER_OBJS),input $(obj)\n)|g' $(BUILD_DIR)/lnk.sub

$(OUTPUT_DIR)/tbg.bin: $(OUTPUT_DIR)/tbg.elf
	wibo "$(KATANA_SDK_DIR)/bin/elf2bin.exe" -s 8c010000 "$(subst /,\\,$<)"
	@if ! echo "$(SHA1_CHECKSUM) *$(OUTPUT_DIR)/tbg.bin" | sha1sum --status -c -; then \
		echo "================" ;\
		echo "Project built :)" ;\
		echo "================" ;\
	else \
		echo "===========================" ;\
		echo "Matching project built! \o/" ;\
		echo "===========================" ;\
	fi

graph: all
	python3 scripts/generate_graph.py

clean:
	rm -rf $(OUTPUT_DIR) $(BUILD_DIR)/lnk.sub

depend:
	makedepend -Y -o .obj -f- $(C_SRCS) 2>/dev/null > Makefile.d
	sed -i 's/^src/$$(OUTPUT_DIR)/' Makefile.d

.PHONY: all graph clean create_dirs $(OUTPUT_DIR)/tbg.bin

include Makefile.d
