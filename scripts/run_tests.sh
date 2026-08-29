set -e

sh4objtest=sh4objtest

ASMSH_FLAGS="-define=UNIT_TESTING=1 -debug -cpu=sh4 -endian=little -sjis"

assemble() {
  local src_file="$1"
  local base_name=$(basename "$src_file" .src)
  local obj_file="build\\output_test\\${base_name}_src.obj"

  wibo "$SHC_BIN/asmsh.exe" $(echo "$src_file"| tr / '\\') -object="$obj_file" $ASMSH_FLAGS
}

compile() {
  local src_file="$1"
  local base_name=$(basename "$src_file" .c)
  local obj_file="build\\output_test\\${base_name}_c.obj"
  local asm_file="build\\output_test\\${base_name}_c.src"

  wibo "$SHC_BIN/shc.exe" $(echo "$src_file" | tr / '\\') -object="$obj_file" -sub=build/shc_testing.sub 

  # Generate ASM file, useful for debugging.
  wibo "$SHC_BIN/shc.exe" $(echo "$src_file" | tr / '\\') -code=asm -object="$asm_file" -sub=build/shc_testing.sub 
}

rm -rf build/output_test build/tmp
mkdir build/output_test build/tmp

# 012324
assemble  src/asm/decompiled/012324_peripheral_support.src
compile  src/012324_peripheral_support.c

# 014f44
assemble  src/asm/decompiled/014f54_text.src
compile  src/014f54_text.c

# 0100bc_sound
assemble  src/asm/decompiled/0100bc_sound.src
compile  src/0100bc_sound.c

# 010fe8_heap
assemble  src/asm/decompiled/010fe8_heap.src
compile  src/010fe8_heap.c

# 015ab8_title
assemble  src/asm/decompiled/015ab8_title.src
compile  src/015ab8_title.c

# 0193c8
assemble  src/asm/decompiled/0193c8_vm_menu.src
compile  src/0193c8_vm_menu.c

# 0207d4
assemble  src/asm/decompiled/0207d4.src
compile  src/0207d4.c

# 020594
assemble  src/asm/decompiled/020594.src
compile  src/020594.c

# 02081c
assemble  src/asm/decompiled/02081c.src
compile  src/02081c.c

# 016c58_prompt
assemble  src/asm/decompiled/016c58_prompt.src
compile  src/016c58_prompt.c

# 012f44_game
assemble  src/asm/decompiled/012f44_game.src
compile  src/012f44_game.c

# 011120
assemble  src/asm/decompiled/011120_asset_queues.src
compile  src/011120_asset_queues.c

# 019e98
assemble  src/asm/decompiled/019e98_main_menu.src
compile  src/019e98_main_menu.c

# 019e98
assemble  src/asm/decompiled/016d2c_course_menu.src
compile  src/016d2c_course_menu.c

# 012504_input
assemble  src/asm/decompiled/012504_input.src
compile  src/012504_input.c

# 016bf4_demo_input
assemble  src/asm/decompiled/016bf4_demo_input.src
compile  src/016bf4_demo_input.c

# 01d290_album
assemble  src/asm/decompiled/01d290_album.src
compile  src/01d290_album.c

# 01d7fc_results
assemble  src/asm/decompiled/01d7fc_results.src
compile  src/01d7fc_results.c

# 013ae8_route_load
assemble  src/asm/decompiled/013ae8_route_load.src
compile  src/013ae8_route_load.c

# 02af78_event
assemble  src/asm/decompiled/02af78_event.src
compile  src/02af78_event.c

# 02c884_bus_stop
assemble  src/asm/decompiled/02c884_bus_stop.src
compile  src/02c884_bus_stop.c

# 026710_traffic
assemble  src/asm/decompiled/026710_traffic.src
compile  src/026710_traffic.c

# 0129cc
assemble  src/asm/decompiled/0129cc_pause.src
compile  src/0129cc_pause.c

# 01614c_debug_menu
assemble  src/asm/decompiled/01614c_debug_menu.src
compile  src/01614c_debug_menu.c

# 018644
assemble  src/asm/decompiled/018644_file_menu.src
compile  src/018644_file_menu.c

# 01a148
assemble  src/asm/decompiled/01a148_option.src
compile  src/01a148_option.c

# 01b19c_system_menu
assemble  src/asm/decompiled/01b19c_system_menu.src
compile  src/01b19c_system_menu.c

# 01bb48_vm_game
assemble  src/asm/decompiled/01bb48_vm_game.src
compile  src/01bb48_vm_game.c

# 01c980_profile_file
assemble  src/asm/decompiled/01c980_profile_file.src
compile  src/01c980_profile_file.c

# 01e27c_practice_menu
assemble  src/asm/decompiled/01e27c_practice_menu.src
compile  src/01e27c_practice_menu.c

# 01f3c0_ending
assemble  src/asm/decompiled/01f3c0_ending.src
compile  src/01f3c0_ending.c

# 01fa78
assemble  src/asm/decompiled/01fa78.src
compile  src/01fa78.c

# 02171c_tile_stream
assemble  src/asm/decompiled/02171c_tile_stream.src
compile  src/02171c_tile_stream.c

# 0222dc_fadecmd
assemble  src/asm/decompiled/0222dc_fadecmd.src
compile  src/0222dc_fadecmd.c

# 022464_fade
assemble  src/asm/decompiled/022464_fade.src
compile  src/022464_fade.c

# 028258_objects
assemble  src/asm/decompiled/028258_objects.src
compile  src/028258_objects.c

# 020914_ground_query
assemble  src/asm/decompiled/020914_ground_query.src
compile  src/020914_ground_query.c

# 020b6c_ground_probe
assemble  src/asm/decompiled/020b6c_ground_probe.src
compile  src/020b6c_ground_probe.c

# 0206f0_intersect
assemble  src/asm/decompiled/0206f0_intersect.src
compile  src/0206f0_intersect.c

# 02786c_vehicle_parts
assemble  src/asm/decompiled/02786c_vehicle_parts.src
compile  src/02786c_vehicle_parts.c

# 02e400_collision
assemble  src/asm/decompiled/02e400_collision.src
compile  src/02e400_collision.c

# 02f0c8
assemble  src/asm/decompiled/02f0c8.src
compile  src/02f0c8.c

# 02b464
assemble  src/asm/decompiled/02b464_drive_points.src
compile  src/02b464_drive_points.c

# 02f320
assemble  src/asm/decompiled/02f320_replay_codec.src
compile  src/02f320_replay_codec.c

# 023310_bus_init
assemble  src/asm/decompiled/023310_bus_init.src
compile  src/023310_bus_init.c

# 022bdc_bus
assemble  src/asm/decompiled/022bdc_bus.src
compile  src/022bdc_bus.c

# 024b4c
assemble  src/asm/decompiled/024b4c_bus_render.src
compile  src/024b4c_bus_render.c

# 023938
assemble  src/asm/decompiled/023938_bus_drive.src
compile  src/023938_bus_drive.c

# 025870
assemble  src/asm/decompiled/025870.src
compile  src/025870.c

# 02e51c
assemble  src/asm/decompiled/02e51c.src
compile  src/02e51c.c

# 027958
assemble  src/asm/decompiled/027958.src
compile  src/027958.c

# 025b98
assemble  src/asm/decompiled/025b98_traffic_drive.src
compile  src/025b98_traffic_drive.c

$sh4objtest suite -s tests.php "$@"
