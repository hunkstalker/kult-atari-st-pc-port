/* GENERADO por tools/gen_cpuwork_costs.py — NO editar a mano.
 * Coste = ciclos del 68000 por llamada, medidos del oráculo (referencia
 * del predecesor) y mapeados a símbolos C del port. Excluidas esperas,
 * retardos dbf ya cobrados por cpu_delay y audio. */
extern void action_bar_hit_test(void);
extern void action_menu(void);
extern void advance_text_entry(void);
extern void anim_bank(void);
extern void anim_bank_build(void);
extern void anim_bank_select(void);
extern void anim_counter(void);
extern void anim_engine_entry(void);
extern void anim_entry_a(void);
extern void anim_entry_b(void);
extern void anim_h1(void);
extern void anim_h2(void);
extern void anim_h4(void);
extern void anim_h6(void);
extern void anim_h7(void);
extern void anim_mode(void);
extern void anim_restore(void);
extern void anim_step(void);
extern void anim_threshold(void);
extern void anim_threshold2(void);
extern void attract(void);
extern void bank_blit_a(void);
extern void bank_blit_b(void);
extern void bit_permute_byte(void);
extern void bit_reverse_word(void);
extern void blit_dispatch_a(void);
extern void blit_dispatch_b(void);
extern void blit_mask_row(void);
extern void blit_mask_rows(void);
extern void blit_mask_setup(void);
extern void blit_rect(void);
extern void blit_rect_vbl(void);
extern void blit_sprite(void);
extern void box_hline(void);
extern void box_hline_back(void);
extern void box_hline_copy(void);
extern void box_vline(void);
extern void box_vline_back(void);
extern void box_vline_copy(void);
extern void class30_apply(void);
extern void class30_dispatch(void);
extern void class_find(void);
extern void clear_flag(void);
extern void clear_globals(void);
extern void clear_object_flags(void);
extern void clock(void);
extern void cmd135(void);
extern void cmd74_wrap(void);
extern void command_state_reset(void);
extern void copy_grid(void);
extern void copy_rows(void);
extern void copy_rows_a(void);
extern void copy_rows_vbl(void);
extern void copy_scratch_rows(void);
extern void copy_screen(void);
extern void copy_screen_a6(void);
extern void copy_screen_rev(void);
extern void crt0_init(void);
extern void cursor_anim_wait(void);
extern void cursor_draw(void);
extern void cursor_restore(void);
extern void cursor_select_a(void);
extern void cursor_select_b(void);
extern void dispatch_a6(void);
extern void dispatch_cmd(void);
extern void draw_hint_icons(void);
extern void draw_hint_icons_wrap(void);
extern void draw_selection_frame(void);
extern void draw_selection_frame_wrap(void);
extern void draw_text_box(void);
extern void draw_text_box_variant(void);
extern void elem_bitmap_build(void);
extern void elem_bitmap_build2(void);
extern void elem_blit(void);
extern void elem_blit_pair(void);
extern void elem_blit_pair_vbl(void);
extern void elem_blit_rev(void);
extern void elem_blit_vbl(void);
extern void elem_class_find(void);
extern void elem_command(void);
extern void elem_list_build(void);
extern void elem_mode_blit(void);
extern void fix_sample_flag(void);
extern void game_loop(void);
extern void gemdos_cnecin(void);
extern void gemdos_fclose(void);
extern void gemdos_fcreate(void);
extern void gemdos_fopen(void);
extern void gemdos_fread(void);
extern void gemdos_fwrite(void);
extern void grid_bands(void);
extern void grid_fill(void);
extern void grid_wait(void);
extern void handle_frame_input(void);
extern void hint_blit_2(void);
extern void hint_blit_3(void);
extern void hint_blit_entry(void);
extern void hint_blit_entry_b(void);
extern void hint_blit_stroke(void);
extern void hint_color(void);
extern void hint_find(void);
extern void hint_icons(void);
extern void hint_icons_wrap(void);
extern void hint_select_off(void);
extern void hint_select_on(void);
extern void hint_slot_alloc(void);
extern void hint_state_a(void);
extern void hint_state_b(void);
extern void hint_table_relocate(void);
extern void hit_test(void);
extern void hit_test_objects(void);
extern void icon_anim_a(void);
extern void icon_anim_b(void);
extern void icon_anim_c(void);
extern void icon_apply(void);
extern void icon_blit_handler(void);
extern void icon_list_build(void);
extern void icon_list_reset(void);
extern void icon_repeat(void);
extern void icon_reveal(void);
extern void icon_reveal_h(void);
extern void icon_reveal_v(void);
extern void icon_ring(void);
extern void icon_ring_loop(void);
extern void icon_scale(void);
extern void icon_scale_loop(void);
extern void icon_state_a(void);
extern void icon_state_b(void);
extern void icon_table(void);
extern void icon_table_entry(void);
extern void ikbd_packet_copy(void);
extern void init_hardware(void);
extern void inline_cmd_walk(void);
extern void load_eche_bank(void);
extern void load_eche_bank_cmd(void);
extern void load_eche_bank_wrap(void);
extern void load_fond(void);
extern void load_named_screen(void);
extern void load_resource(void);
extern void load_resource_list(void);
extern void load_resource_list_a(void);
extern void load_resource_list_b(void);
extern void load_room(void);
extern void load_scena(void);
extern void mask_restore_rows(void);
extern void mask_row_blit(void);
extern void mask_rows_handler(void);
extern void masked_save(void);
extern void masked_save_blit(void);
extern void masked_save_row(void);
extern void masked_word4(void);
extern void measure_word(void);
extern void measure_words(void);
extern void mfp_setup(void);
extern void mode_flag_wrap(void);
extern void mode_queue(void);
extern void mode_queue_wrap(void);
extern void music_cmd(void);
extern void music_cmd_table(void);
extern void music_h1(void);
extern void music_h2(void);
extern void music_h3(void);
extern void music_h4(void);
extern void music_h5(void);
extern void music_install(void);
extern void music_load(void);
extern void music_note(void);
extern void music_silence(void);
extern void music_stop(void);
extern void music_stop2(void);
extern void music_tick(void);
extern void name_prompt(void);
extern void noop_11014(void);
extern void noop_135e0(void);
extern void noop_160c0(void);
extern void noop_16dba(void);
extern void noop_19312(void);
extern void obj_action(void);
extern void obj_action_select(void);
extern void obj_anim(void);
extern void obj_clear_anim(void);
extern void obj_consume(void);
extern void obj_dec(void);
extern void obj_find(void);
extern void obj_find_class(void);
extern void obj_find_masked(void);
extern void obj_find_or_exit(void);
extern void obj_find_setup(void);
extern void obj_list_a(void);
extern void obj_list_b(void);
extern void obj_list_iter(void);
extern void obj_mark(void);
extern void obj_name(void);
extern void obj_name_clean(void);
extern void obj_name_snd(void);
extern void obj_name_table(void);
extern void obj_name_track(void);
extern void obj_refresh(void);
extern void obj_seal(void);
extern void obj_text_box(void);
extern void obj_wait(void);
extern void obj_wait_a0(void);
extern void obj_wait_cmd(void);
extern void obj_wait_cmd_a(void);
extern void obj_wait_cmd_b(void);
extern void obj_wait_cmd_snd(void);
extern void obj_wait_hit(void);
extern void object_info_card(void);
extern void object_info_wait(void);
extern void or_rows(void);
extern void overlay_a_draw(void);
extern void overlay_a_frame(void);
extern void overlay_b_draw(void);
extern void overlay_b_frame(void);
extern void overlay_compose_masked(void);
extern void overlay_compose_masked_wrap(void);
extern void overlay_compose_planes(void);
extern void overlay_compose_planes32(void);
extern void overlay_compose_planes32_wrap(void);
extern void overlay_compose_planes_wrap(void);
extern void overlay_compose_shift(void);
extern void overlay_compose_shift_wrap(void);
extern void overlay_dispatch_1(void);
extern void overlay_dispatch_2(void);
extern void overlay_dispatch_3(void);
extern void overlay_dispatch_4(void);
extern void overlay_dispatch_5(void);
extern void overlay_dispatch_6(void);
extern void overlay_menu(void);
extern void pal_next_byte(void);
extern void pal_next_word(void);
extern void pal_threshold(void);
extern void palette_cmd(void);
extern void palette_cycle(void);
extern void pattern_fill(void);
extern void portrait_intro(void);
extern void position_text(void);
extern void prepare_room(void);
extern void print_char(void);
extern void print_line(void);
extern void print_line_center(void);
extern void print_spaces(void);
extern void print_word(void);
extern void print_words_fit(void);
extern void prompt(void);
extern void prompt_wait(void);
extern void prompt_wait_entry(void);
extern void psg_shadow(void);
extern void psg_trap3(void);
extern void psg_write_verify(void);
extern void rect_hit_test(void);
extern void rect_hit_test_a(void);
extern void reset_clear(void);
extern void restore_rect(void);
extern void reveal_box_a(void);
extern void reveal_box_b(void);
extern void reveal_box_build(void);
extern void reveal_box_wait_a(void);
extern void reveal_box_wait_b(void);
extern void reveal_diamond(void);
extern void reveal_dot(void);
extern void reveal_line(void);
extern void reveal_lines_h(void);
extern void reveal_lines_v(void);
extern void reveal_pixel(void);
extern void reveal_region(void);
extern void room_reset(void);
extern void rot_action(void);
extern void row_blit(void);
extern void row_blit_a(void);
extern void row_blit_b(void);
extern void row_palette(void);
extern void row_transition(void);
extern void run_command(void);
extern void run_command_keep_sp(void);
extern void run_text_entry_1(void);
extern void run_text_entry_ex(void);
extern void run_text_table(void);
extern void sample_entry(void);
extern void sample_find(void);
extern void sample_find_body(void);
extern void sample_find_wrap(void);
extern void sample_list(void);
extern void sample_mark_a(void);
extern void sample_mark_b(void);
extern void sample_mark_c(void);
extern void sample_menu(void);
extern void sample_menu_a(void);
extern void sample_menu_b(void);
extern void sample_menu_entry_a(void);
extern void sample_menu_entry_b(void);
extern void sample_pair_a(void);
extern void sample_queue(void);
extern void sample_select(void);
extern void sample_select2(void);
extern void sample_walk(void);
extern void save_clear_bin(void);
extern void save_rect(void);
extern void save_rect_body(void);
extern void save_room(void);
extern void save_scena(void);
extern void scale_16_16(void);
extern void scale_anim(void);
extern void scale_blit(void);
extern void scale_list_build(void);
extern void scale_loop(void);
extern void scale_prep(void);
extern void scene_mode_apply(void);
extern void scene_mode_draw(void);
extern void scene_mode_draw_wrap(void);
extern void scene_mode_load(void);
extern void scene_mode_set(void);
extern void scene_text_box(void);
extern void screen_address(void);
extern void screen_address_raw(void);
extern void screen_clean(void);
extern void screen_offset(void);
extern void script_alu(void);
extern void script_arg(void);
extern void script_call(void);
extern void script_expr(void);
extern void script_jump_zero(void);
extern void script_pop(void);
extern void script_push(void);
extern void script_step(void);
extern void script_store(void);
extern void scroll_down(void);
extern void scroll_up(void);
extern void sel_timer_a(void);
extern void sel_timer_b(void);
extern void set_cursor_clip(void);
extern void set_palette(void);
extern void set_palette_advance(void);
extern void set_palette_current(void);
extern void set_palette_entry(void);
extern void set_palette_from_entry(void);
extern void sound_isr_back(void);
extern void sound_isr_fwd(void);
extern void sound_isr_load(void);
extern void sound_off(void);
extern void sound_path(void);
extern void sound_play_a(void);
extern void sound_play_b(void);
extern void sound_stop(void);
extern void sound_stop_supexec(void);
extern void sound_timer_start(void);
extern void sound_timer_start_body(void);
extern void sound_timer_stop(void);
extern void sound_vector(void);
extern void supexec_a(void);
extern void supexec_b(void);
extern void supexec_wrap(void);
extern void swap_screens(void);
extern void text_blit_body_a(void);
extern void text_blit_entry(void);
extern void text_blit_fwd(void);
extern void text_blit_fwd_a3(void);
extern void text_blit_ovl_a(void);
extern void text_blit_ovl_b(void);
extern void text_blit_rev(void);
extern void text_blit_rev_entry(void);
extern void text_blit_setup(void);
extern void text_block_build(void);
extern void text_box_body(void);
extern void text_box_icon(void);
extern void text_end(void);
extern void text_entry_blit(void);
extern void text_index_arg(void);
extern void text_table_entry(void);
extern void text_table_paint(void);
extern void text_transition(void);
extern void text_window(void);
extern void text_window_entry(void);
extern void textform_build(void);
extern void textform_draw(void);
extern void textform_fwd(void);
extern void textform_fwd_entry(void);
extern void textform_rev(void);
extern void textform_rev_entry(void);
extern void track5_apply(void);
extern void track5_body(void);
extern void track_advance(void);
extern void track_icon_anim(void);
extern void track_name_print(void);
extern void track_select_a(void);
extern void track_select_b(void);
extern void track_walk(void);
extern void update_icon_list(void);
extern void update_icon_list_wrap(void);
extern void use_object(void);
extern void view_change(void);
extern void view_mode_cycle(void);
extern void vm_run(void);
extern void wait_button_a(void);
extern void wait_button_b(void);
extern void wait_frame(void);
extern void wait_frame_cursor(void);
extern void wait_vbl(void);
extern void wait_vbl_even(void);
extern void walk_entry(void);
extern void wipe_72(void);
extern void wipe_a(void);
extern void wipe_b(void);
extern void wipe_c(void);
extern void wipe_d(void);
extern void wipe_e(void);
extern void wipe_f(void);
extern void wipe_rows(void);
extern void wrap_line(void);
extern void write_named_file(void);
extern void xbios_setpalette(void);
extern void ym_reset(void);
extern void ym_write(void);

static const cpuwork_sym k_cpuwork_costs[] = {
  { (void *)draw_text_box_variant, 0x10440, 502 },
  { (void *)vm_run, 0x10630, 171 },
  { (void *)load_eche_bank, 0x10682, 272 },
  { (void *)load_resource_list_a, 0x106e2, 22 },
  { (void *)load_resource_list_b, 0x106ea, 5510 },
  { (void *)text_end, 0x10768, 1518 },
  { (void *)print_words_fit, 0x1090e, 367 },
  { (void *)measure_word, 0x10946, 534 },
  { (void *)measure_words, 0x10998, 452 },
  { (void *)wrap_line, 0x109bc, 1280 },
  { (void *)print_line, 0x10b24, 85 },
  { (void *)print_spaces, 0x10b46, 36 },
  { (void *)text_box_icon, 0x10dfa, 324 },
  { (void *)text_box_body, 0x10e66, 7933 },
  { (void *)text_index_arg, 0x10fb4, 43700 },
  { (void *)screen_offset, 0x10ff4, 114 },
  { (void *)noop_11014, 0x11014, 16 },
  { (void *)text_blit_ovl_b, 0x11020, 12 },
  { (void *)text_blit_entry, 0x11026, 30 },
  { (void *)text_blit_body_a, 0x11030, 48 },
  { (void *)text_blit_setup, 0x1103e, 38 },
  { (void *)text_blit_fwd_a3, 0x1104a, 6158 },
  { (void *)overlay_compose_planes32_wrap, 0x114b0, 26 },
  { (void *)overlay_compose_planes32, 0x114ba, 74953 },
  { (void *)row_blit_a, 0x11ece, 4188 },
  { (void *)row_blit_b, 0x11ee8, 4188 },
  { (void *)blit_dispatch_a, 0x11f02, 162 },
  { (void *)blit_dispatch_b, 0x11f2e, 176 },
  { (void *)overlay_dispatch_1, 0x11f64, 46 },
  { (void *)overlay_dispatch_2, 0x11f98, 202 },
  { (void *)hint_slot_alloc, 0x120a8, 260 },
  { (void *)hint_find, 0x1213a, 259 },
  { (void *)mask_restore_rows, 0x1215e, 1792 },
  { (void *)prompt_wait_entry, 0x1228a, 42 },
  { (void *)prompt_wait, 0x12292, 1109 },
  { (void *)prompt, 0x122ee, 159 },
  { (void *)gemdos_cnecin, 0x12324, 70 },
  { (void *)hint_blit_2, 0x12368, 266 },
  { (void *)hint_blit_3, 0x12384, 233 },
  { (void *)copy_scratch_rows, 0x123a0, 14486 },
  { (void *)blit_mask_row, 0x123c0, 528 },
  { (void *)blit_mask_rows, 0x12466, 15722 },
  { (void *)blit_rect_vbl, 0x124da, 156 },
  { (void *)restore_rect, 0x124f2, 94 },
  { (void *)blit_rect, 0x12510, 67689 },
  { (void *)save_rect, 0x1258a, 12 },
  { (void *)save_rect_body, 0x12590, 101346 },
  { (void *)blit_mask_setup, 0x12698, 252 },
  { (void *)grid_fill, 0x126e0, 223810 },
  { (void *)elem_command, 0x128a4, 1742 },
  { (void *)elem_bitmap_build, 0x1296c, 19796 },
  { (void *)elem_list_build, 0x129c8, 48 },
  { (void *)text_block_build, 0x129d6, 283 },
  { (void *)elem_blit, 0x12ad6, 16347 },
  { (void *)elem_blit_rev, 0x12b04, 9974 },
  { (void *)bit_permute_byte, 0x12b40, 110 },
  { (void *)masked_word4, 0x12fb6, 244 },
  { (void *)box_hline, 0x12ffa, 22804 },
  { (void *)box_vline, 0x13216, 478 },
  { (void *)script_pop, 0x13522, 182 },
  { (void *)set_palette, 0x135da, 34 },
  { (void *)crt0_init, 0x135e2, 726 },
  { (void *)anim_entry_a, 0x159d2, 50 },
  { (void *)anim_entry_b, 0x159e2, 2332 },
  { (void *)anim_bank_select, 0x15a90, 114 },
  { (void *)anim_bank_build, 0x15abe, 38 },
  { (void *)anim_bank, 0x15ac8, 14155 },
  { (void *)anim_h2, 0x15bb2, 30 },
  { (void *)anim_h4, 0x15bbc, 46 },
  { (void *)anim_h1, 0x15bcc, 4806 },
  { (void *)anim_counter, 0x15cd8, 152 },
  { (void *)anim_restore, 0x15d7e, 136 },
  { (void *)script_arg, 0x15f6c, 137 },
  { (void *)script_alu, 0x15fd4, 80 },
  { (void *)script_expr, 0x1606e, 126 },
  { (void *)script_jump_zero, 0x160aa, 64 },
  { (void *)pal_next_byte, 0x16210, 100 },
  { (void *)sound_stop_supexec, 0x18f84, 58 },
  { (void *)ym_reset, 0x19282, 1786 },
};
static const unsigned k_cpuwork_ncosts = 79;

/* Marcas (F12): todas las funciones mapeadas, coste 0. */
static const cpuwork_sym k_cpuwork_marks[] = {
  { (void *)game_loop, 0x101be, 0 },
  { (void *)handle_frame_input, 0x1031c, 0 },
  { (void *)dispatch_a6, 0x10390, 0 },
  { (void *)run_command, 0x103a8, 0 },
  { (void *)run_command_keep_sp, 0x103ae, 0 },
  { (void *)draw_text_box, 0x1042c, 0 },
  { (void *)draw_text_box_variant, 0x10440, 0 },
  { (void *)inline_cmd_walk, 0x104f4, 0 },
  { (void *)wait_frame_cursor, 0x10522, 0 },
  { (void *)cursor_draw, 0x10526, 0 },
  { (void *)cursor_restore, 0x1060a, 0 },
  { (void *)vm_run, 0x10630, 0 },
  { (void *)load_eche_bank_cmd, 0x10668, 0 },
  { (void *)load_eche_bank_wrap, 0x1066e, 0 },
  { (void *)load_eche_bank, 0x10682, 0 },
  { (void *)load_resource_list_a, 0x106e2, 0 },
  { (void *)load_resource_list_b, 0x106ea, 0 },
  { (void *)load_resource_list, 0x106f8, 0 },
  { (void *)text_end, 0x10768, 0 },
  { (void *)load_resource, 0x1076a, 0 },
  { (void *)save_clear_bin, 0x1077e, 0 },
  { (void *)write_named_file, 0x107a2, 0 },
  { (void *)gemdos_fopen, 0x107b6, 0 },
  { (void *)gemdos_fcreate, 0x107d0, 0 },
  { (void *)gemdos_fread, 0x107e8, 0 },
  { (void *)gemdos_fwrite, 0x10808, 0 },
  { (void *)gemdos_fclose, 0x10828, 0 },
  { (void *)print_char, 0x10838, 0 },
  { (void *)print_word, 0x108a2, 0 },
  { (void *)print_words_fit, 0x1090e, 0 },
  { (void *)measure_word, 0x10946, 0 },
  { (void *)measure_words, 0x10998, 0 },
  { (void *)wrap_line, 0x109bc, 0 },
  { (void *)overlay_a_draw, 0x109fe, 0 },
  { (void *)overlay_a_frame, 0x10a54, 0 },
  { (void *)overlay_b_draw, 0x10a88, 0 },
  { (void *)overlay_b_frame, 0x10acc, 0 },
  { (void *)print_line_center, 0x10aee, 0 },
  { (void *)print_line, 0x10b24, 0 },
  { (void *)print_spaces, 0x10b46, 0 },
  { (void *)text_window_entry, 0x10b52, 0 },
  { (void *)text_window, 0x10b6c, 0 },
  { (void *)text_box_icon, 0x10dfa, 0 },
  { (void *)text_box_body, 0x10e66, 0 },
  { (void *)advance_text_entry, 0x10f8e, 0 },
  { (void *)text_index_arg, 0x10fb4, 0 },
  { (void *)position_text, 0x10fc4, 0 },
  { (void *)screen_address, 0x10fec, 0 },
  { (void *)screen_address_raw, 0x10fee, 0 },
  { (void *)screen_offset, 0x10ff4, 0 },
  { (void *)noop_11014, 0x11014, 0 },
  { (void *)text_blit_ovl_a, 0x11016, 0 },
  { (void *)text_blit_ovl_b, 0x11020, 0 },
  { (void *)text_blit_entry, 0x11026, 0 },
  { (void *)text_blit_body_a, 0x11030, 0 },
  { (void *)text_blit_setup, 0x1103e, 0 },
  { (void *)text_blit_fwd_a3, 0x1104a, 0 },
  { (void *)text_blit_fwd, 0x11050, 0 },
  { (void *)text_blit_rev_entry, 0x110da, 0 },
  { (void *)text_blit_rev, 0x11102, 0 },
  { (void *)bit_reverse_word, 0x11196, 0 },
  { (void *)overlay_compose_planes_wrap, 0x111ac, 0 },
  { (void *)overlay_compose_planes, 0x111b6, 0 },
  { (void *)overlay_compose_planes32_wrap, 0x114b0, 0 },
  { (void *)overlay_compose_planes32, 0x114ba, 0 },
  { (void *)overlay_compose_masked_wrap, 0x117de, 0 },
  { (void *)overlay_compose_masked, 0x117fe, 0 },
  { (void *)overlay_compose_shift_wrap, 0x11ada, 0 },
  { (void *)overlay_compose_shift, 0x11afa, 0 },
  { (void *)scroll_up, 0x11dee, 0 },
  { (void *)scroll_down, 0x11e5e, 0 },
  { (void *)row_blit_a, 0x11ece, 0 },
  { (void *)row_blit_b, 0x11ee8, 0 },
  { (void *)blit_dispatch_a, 0x11f02, 0 },
  { (void *)blit_dispatch_b, 0x11f2e, 0 },
  { (void *)overlay_dispatch_1, 0x11f64, 0 },
  { (void *)overlay_dispatch_2, 0x11f98, 0 },
  { (void *)overlay_dispatch_3, 0x11fd8, 0 },
  { (void *)overlay_dispatch_4, 0x1200a, 0 },
  { (void *)overlay_dispatch_5, 0x12044, 0 },
  { (void *)overlay_dispatch_6, 0x12076, 0 },
  { (void *)hint_slot_alloc, 0x120a8, 0 },
  { (void *)hint_select_on, 0x120dc, 0 },
  { (void *)hint_select_off, 0x120e4, 0 },
  { (void *)hint_find, 0x1213a, 0 },
  { (void *)mask_restore_rows, 0x1215e, 0 },
  { (void *)mask_row_blit, 0x12190, 0 },
  { (void *)copy_screen_rev, 0x121b6, 0 },
  { (void *)copy_screen_a6, 0x121c4, 0 },
  { (void *)copy_screen, 0x121c6, 0 },
  { (void *)swap_screens, 0x121de, 0 },
  { (void *)icon_blit_handler, 0x121fa, 0 },
  { (void *)hint_blit_entry, 0x1222e, 0 },
  { (void *)wait_button_a, 0x12240, 0 },
  { (void *)wait_button_b, 0x1224e, 0 },
  { (void *)prompt_wait_entry, 0x1228a, 0 },
  { (void *)prompt_wait, 0x12292, 0 },
  { (void *)prompt, 0x122ee, 0 },
  { (void *)gemdos_cnecin, 0x12324, 0 },
  { (void *)hint_blit_stroke, 0x1232e, 0 },
  { (void *)hint_blit_2, 0x12368, 0 },
  { (void *)hint_blit_3, 0x12384, 0 },
  { (void *)copy_scratch_rows, 0x123a0, 0 },
  { (void *)blit_mask_row, 0x123c0, 0 },
  { (void *)mask_rows_handler, 0x12436, 0 },
  { (void *)blit_mask_rows, 0x12466, 0 },
  { (void *)blit_rect_vbl, 0x124da, 0 },
  { (void *)restore_rect, 0x124f2, 0 },
  { (void *)blit_sprite, 0x124f8, 0 },
  { (void *)blit_rect, 0x12510, 0 },
  { (void *)save_rect, 0x1258a, 0 },
  { (void *)save_rect_body, 0x12590, 0 },
  { (void *)blit_mask_setup, 0x12698, 0 },
  { (void *)grid_fill, 0x126e0, 0 },
  { (void *)pattern_fill, 0x1276c, 0 },
  { (void *)masked_save, 0x127e0, 0 },
  { (void *)masked_save_blit, 0x127f0, 0 },
  { (void *)masked_save_row, 0x127fa, 0 },
  { (void *)elem_class_find, 0x1285a, 0 },
  { (void *)elem_command, 0x128a4, 0 },
  { (void *)walk_entry, 0x12950, 0 },
  { (void *)elem_bitmap_build, 0x1296c, 0 },
  { (void *)elem_list_build, 0x129c8, 0 },
  { (void *)text_block_build, 0x129d6, 0 },
  { (void *)elem_bitmap_build2, 0x12a4a, 0 },
  { (void *)elem_blit, 0x12ad6, 0 },
  { (void *)elem_blit_rev, 0x12b04, 0 },
  { (void *)bit_permute_byte, 0x12b40, 0 },
  { (void *)view_mode_cycle, 0x12ce4, 0 },
  { (void *)elem_blit_pair, 0x12d96, 0 },
  { (void *)elem_blit_pair_vbl, 0x12dd8, 0 },
  { (void *)reveal_box_a, 0x12e08, 0 },
  { (void *)reveal_region, 0x12e2e, 0 },
  { (void *)reveal_box_b, 0x12ea2, 0 },
  { (void *)reveal_dot, 0x12ef6, 0 },
  { (void *)reveal_pixel, 0x12f28, 0 },
  { (void *)masked_word4, 0x12fb6, 0 },
  { (void *)hint_blit_entry_b, 0x12fea, 0 },
  { (void *)box_hline_back, 0x12fee, 0 },
  { (void *)box_hline, 0x12ffa, 0 },
  { (void *)box_hline_copy, 0x130a4, 0 },
  { (void *)box_vline_copy, 0x13162, 0 },
  { (void *)box_vline_back, 0x1320a, 0 },
  { (void *)box_vline, 0x13216, 0 },
  { (void *)reveal_box_build, 0x132aa, 0 },
  { (void *)reveal_box_wait_a, 0x132ec, 0 },
  { (void *)reveal_box_wait_b, 0x1332e, 0 },
  { (void *)reveal_diamond, 0x13370, 0 },
  { (void *)icon_reveal, 0x13474, 0 },
  { (void *)icon_reveal_h, 0x13490, 0 },
  { (void *)icon_reveal_v, 0x134b2, 0 },
  { (void *)elem_blit_vbl, 0x134da, 0 },
  { (void *)copy_grid, 0x134e4, 0 },
  { (void *)script_push, 0x1350c, 0 },
  { (void *)script_pop, 0x13522, 0 },
  { (void *)script_call, 0x13552, 0 },
  { (void *)load_fond, 0x13568, 0 },
  { (void *)load_named_screen, 0x13572, 0 },
  { (void *)wait_vbl, 0x13594, 0 },
  { (void *)set_palette, 0x135da, 0 },
  { (void *)noop_135e0, 0x135e0, 0 },
  { (void *)crt0_init, 0x135e2, 0 },
  { (void *)ikbd_packet_copy, 0x13686, 0 },
  { (void *)wait_frame, 0x13698, 0 },
  { (void *)scale_prep, 0x13702, 0 },
  { (void *)scale_loop, 0x13740, 0 },
  { (void *)scale_anim, 0x138a6, 0 },
  { (void *)scale_list_build, 0x1398e, 0 },
  { (void *)scale_16_16, 0x13a18, 0 },
  { (void *)scale_blit, 0x13bc2, 0 },
  { (void *)reveal_line, 0x13d0e, 0 },
  { (void *)reveal_lines_h, 0x13e14, 0 },
  { (void *)reveal_lines_v, 0x13eae, 0 },
  { (void *)text_table_paint, 0x13f2a, 0 },
  { (void *)text_table_entry, 0x13f4c, 0 },
  { (void *)run_text_table, 0x13f56, 0 },
  { (void *)scene_text_box, 0x13f86, 0 },
  { (void *)run_text_entry_ex, 0x14138, 0 },
  { (void *)text_entry_blit, 0x141dc, 0 },
  { (void *)run_text_entry_1, 0x141ea, 0 },
  { (void *)textform_fwd_entry, 0x141f4, 0 },
  { (void *)textform_fwd, 0x141fa, 0 },
  { (void *)textform_rev_entry, 0x14246, 0 },
  { (void *)textform_rev, 0x1424c, 0 },
  { (void *)textform_build, 0x142cc, 0 },
  { (void *)textform_draw, 0x143e2, 0 },
  { (void *)cmd74_wrap, 0x1442c, 0 },
  { (void *)scene_mode_set, 0x1443e, 0 },
  { (void *)scene_mode_load, 0x14496, 0 },
  { (void *)command_state_reset, 0x14588, 0 },
  { (void *)scene_mode_draw_wrap, 0x14602, 0 },
  { (void *)scene_mode_apply, 0x1460a, 0 },
  { (void *)scene_mode_draw, 0x146d0, 0 },
  { (void *)elem_mode_blit, 0x14748, 0 },
  { (void *)rect_hit_test_a, 0x1478a, 0 },
  { (void *)rect_hit_test, 0x14798, 0 },
  { (void *)cursor_select_a, 0x147c2, 0 },
  { (void *)cursor_select_b, 0x147ce, 0 },
  { (void *)hit_test, 0x14832, 0 },
  { (void *)hit_test_objects, 0x14886, 0 },
  { (void *)class_find, 0x14926, 0 },
  { (void *)action_bar_hit_test, 0x14966, 0 },
  { (void *)action_menu, 0x149da, 0 },
  { (void *)set_cursor_clip, 0x14a98, 0 },
  { (void *)sample_menu_a, 0x14ac6, 0 },
  { (void *)fix_sample_flag, 0x14bdc, 0 },
  { (void *)sample_menu_b, 0x14bfa, 0 },
  { (void *)sample_menu_entry_a, 0x14d76, 0 },
  { (void *)sample_menu_entry_b, 0x14d90, 0 },
  { (void *)sample_menu, 0x14db2, 0 },
  { (void *)overlay_menu, 0x14e66, 0 },
  { (void *)cmd135, 0x14ee8, 0 },
  { (void *)anim_threshold, 0x14f24, 0 },
  { (void *)anim_threshold2, 0x14f5e, 0 },
  { (void *)grid_wait, 0x14f74, 0 },
  { (void *)obj_action_select, 0x15052, 0 },
  { (void *)obj_action, 0x150ae, 0 },
  { (void *)clear_object_flags, 0x15192, 0 },
  { (void *)row_transition, 0x151b0, 0 },
  { (void *)cursor_anim_wait, 0x152ee, 0 },
  { (void *)palette_cmd, 0x15382, 0 },
  { (void *)name_prompt, 0x15428, 0 },
  { (void *)sample_select, 0x15492, 0 },
  { (void *)sample_select2, 0x15510, 0 },
  { (void *)mode_queue, 0x15530, 0 },
  { (void *)mode_flag_wrap, 0x15588, 0 },
  { (void *)load_room, 0x15594, 0 },
  { (void *)mode_queue_wrap, 0x155b8, 0 },
  { (void *)save_room, 0x155c4, 0 },
  { (void *)clock, 0x155dc, 0 },
  { (void *)icon_ring, 0x1566a, 0 },
  { (void *)icon_ring_loop, 0x15888, 0 },
  { (void *)copy_rows_a, 0x15934, 0 },
  { (void *)copy_rows, 0x15944, 0 },
  { (void *)row_palette, 0x159bc, 0 },
  { (void *)anim_entry_a, 0x159d2, 0 },
  { (void *)anim_entry_b, 0x159e2, 0 },
  { (void *)anim_bank_select, 0x15a90, 0 },
  { (void *)anim_bank_build, 0x15abe, 0 },
  { (void *)anim_bank, 0x15ac8, 0 },
  { (void *)anim_h7, 0x15b94, 0 },
  { (void *)anim_h6, 0x15ba6, 0 },
  { (void *)anim_h2, 0x15bb2, 0 },
  { (void *)anim_h4, 0x15bbc, 0 },
  { (void *)anim_h1, 0x15bcc, 0 },
  { (void *)anim_counter, 0x15cd8, 0 },
  { (void *)anim_restore, 0x15d7e, 0 },
  { (void *)anim_engine_entry, 0x15da6, 0 },
  { (void *)script_arg, 0x15f6c, 0 },
  { (void *)script_alu, 0x15fd4, 0 },
  { (void *)script_expr, 0x1606e, 0 },
  { (void *)script_store, 0x16088, 0 },
  { (void *)script_jump_zero, 0x160aa, 0 },
  { (void *)noop_160c0, 0x160c0, 0 },
  { (void *)pal_next_byte, 0x16210, 0 },
  { (void *)pal_next_word, 0x16230, 0 },
  { (void *)hint_color, 0x16250, 0 },
  { (void *)pal_threshold, 0x1627a, 0 },
  { (void *)set_palette_from_entry, 0x162ce, 0 },
  { (void *)set_palette_advance, 0x162d8, 0 },
  { (void *)set_palette_current, 0x162da, 0 },
  { (void *)set_palette_entry, 0x162e2, 0 },
  { (void *)xbios_setpalette, 0x16300, 0 },
  { (void *)palette_cycle, 0x16310, 0 },
  { (void *)icon_table, 0x16460, 0 },
  { (void *)icon_table_entry, 0x164a4, 0 },
  { (void *)icon_state_a, 0x164d4, 0 },
  { (void *)icon_state_b, 0x164f6, 0 },
  { (void *)hint_icons_wrap, 0x16512, 0 },
  { (void *)hint_icons, 0x16514, 0 },
  { (void *)icon_apply, 0x1652e, 0 },
  { (void *)draw_hint_icons_wrap, 0x16554, 0 },
  { (void *)draw_hint_icons, 0x16556, 0 },
  { (void *)update_icon_list_wrap, 0x16576, 0 },
  { (void *)update_icon_list, 0x16582, 0 },
  { (void *)obj_list_iter, 0x16636, 0 },
  { (void *)obj_list_a, 0x166da, 0 },
  { (void *)obj_list_b, 0x166ec, 0 },
  { (void *)obj_wait, 0x167aa, 0 },
  { (void *)obj_wait_hit, 0x167be, 0 },
  { (void *)obj_wait_a0, 0x1680c, 0 },
  { (void *)reset_clear, 0x1682a, 0 },
  { (void *)obj_name, 0x1687e, 0 },
  { (void *)obj_name_snd, 0x168a0, 0 },
  { (void *)obj_name_clean, 0x168d4, 0 },
  { (void *)obj_name_track, 0x16902, 0 },
  { (void *)obj_name_table, 0x1695a, 0 },
  { (void *)obj_wait_cmd_a, 0x1699c, 0 },
  { (void *)obj_wait_cmd_b, 0x169a4, 0 },
  { (void *)obj_wait_cmd_snd, 0x169ac, 0 },
  { (void *)obj_wait_cmd, 0x169b4, 0 },
  { (void *)portrait_intro, 0x16a20, 0 },
  { (void *)sample_entry, 0x16ab6, 0 },
  { (void *)grid_bands, 0x16abe, 0 },
  { (void *)or_rows, 0x16b2e, 0 },
  { (void *)row_blit, 0x16b76, 0 },
  { (void *)wipe_72, 0x16b9c, 0 },
  { (void *)wipe_a, 0x16c00, 0 },
  { (void *)wipe_b, 0x16c48, 0 },
  { (void *)wipe_rows, 0x16c6c, 0 },
  { (void *)wipe_c, 0x16ca6, 0 },
  { (void *)wipe_d, 0x16ce2, 0 },
  { (void *)wait_vbl_even, 0x16d1e, 0 },
  { (void *)wipe_e, 0x16d3c, 0 },
  { (void *)wipe_f, 0x16d78, 0 },
  { (void *)noop_16dba, 0x16dba, 0 },
  { (void *)text_transition, 0x16dbc, 0 },
  { (void *)object_info_card, 0x16dec, 0 },
  { (void *)load_scena, 0x16ed8, 0 },
  { (void *)hint_table_relocate, 0x16f90, 0 },
  { (void *)save_scena, 0x16fb4, 0 },
  { (void *)hint_state_a, 0x17094, 0 },
  { (void *)sel_timer_a, 0x171b4, 0 },
  { (void *)track_select_a, 0x171fa, 0 },
  { (void *)clear_flag, 0x172ba, 0 },
  { (void *)hint_state_b, 0x172ca, 0 },
  { (void *)sel_timer_b, 0x173ae, 0 },
  { (void *)track_select_b, 0x173f2, 0 },
  { (void *)clear_globals, 0x17472, 0 },
  { (void *)rot_action, 0x17480, 0 },
  { (void *)obj_seal, 0x175f6, 0 },
  { (void *)obj_clear_anim, 0x17678, 0 },
  { (void *)track_icon_anim, 0x176c0, 0 },
  { (void *)obj_find, 0x1778e, 0 },
  { (void *)obj_find_masked, 0x17792, 0 },
  { (void *)obj_find_class, 0x177bc, 0 },
  { (void *)obj_find_setup, 0x177e4, 0 },
  { (void *)obj_find_or_exit, 0x1780c, 0 },
  { (void *)obj_anim, 0x17856, 0 },
  { (void *)obj_consume, 0x178cc, 0 },
  { (void *)obj_text_box, 0x17930, 0 },
  { (void *)class30_apply, 0x179a2, 0 },
  { (void *)view_change, 0x17a1a, 0 },
  { (void *)obj_refresh, 0x17c42, 0 },
  { (void *)use_object, 0x17ca4, 0 },
  { (void *)script_step, 0x17e3c, 0 },
  { (void *)class30_dispatch, 0x18128, 0 },
  { (void *)track5_apply, 0x1816e, 0 },
  { (void *)obj_mark, 0x1817a, 0 },
  { (void *)track5_body, 0x18192, 0 },
  { (void *)room_reset, 0x181f0, 0 },
  { (void *)icon_list_reset, 0x182f0, 0 },
  { (void *)icon_list_build, 0x18316, 0 },
  { (void *)icon_scale, 0x18334, 0 },
  { (void *)icon_scale_loop, 0x1834a, 0 },
  { (void *)prepare_room, 0x1842c, 0 },
  { (void *)sample_mark_a, 0x184cc, 0 },
  { (void *)sample_mark_b, 0x184d6, 0 },
  { (void *)sample_mark_c, 0x1851a, 0 },
  { (void *)sample_walk, 0x18538, 0 },
  { (void *)sample_find_wrap, 0x1857a, 0 },
  { (void *)sample_find_body, 0x1857c, 0 },
  { (void *)track_name_print, 0x185e6, 0 },
  { (void *)sample_find, 0x186b6, 0 },
  { (void *)dispatch_cmd, 0x186ee, 0 },
  { (void *)track_walk, 0x1870a, 0 },
  { (void *)anim_step, 0x18740, 0 },
  { (void *)copy_rows_vbl, 0x1881e, 0 },
  { (void *)bank_blit_a, 0x18888, 0 },
  { (void *)bank_blit_b, 0x18890, 0 },
  { (void *)anim_mode, 0x188ce, 0 },
  { (void *)icon_repeat, 0x18944, 0 },
  { (void *)object_info_wait, 0x189aa, 0 },
  { (void *)track_advance, 0x18a10, 0 },
  { (void *)screen_clean, 0x18a30, 0 },
  { (void *)attract, 0x18a98, 0 },
  { (void *)icon_anim_a, 0x18c8e, 0 },
  { (void *)icon_anim_b, 0x18cb2, 0 },
  { (void *)icon_anim_c, 0x18ce0, 0 },
  { (void *)obj_dec, 0x18cfc, 0 },
  { (void *)draw_selection_frame_wrap, 0x18d26, 0 },
  { (void *)draw_selection_frame, 0x18d28, 0 },
  { (void *)sound_path, 0x18dc0, 0 },
  { (void *)sample_pair_a, 0x18e06, 0 },
  { (void *)sample_list, 0x18e12, 0 },
  { (void *)sample_queue, 0x18f0c, 0 },
  { (void *)supexec_wrap, 0x18f6e, 0 },
  { (void *)supexec_b, 0x18f7a, 0 },
  { (void *)sound_stop_supexec, 0x18f84, 0 },
  { (void *)sound_play_a, 0x18f8e, 0 },
  { (void *)sound_play_b, 0x18f98, 0 },
  { (void *)supexec_a, 0x18fa2, 0 },
  { (void *)sound_stop, 0x18fa4, 0 },
  { (void *)sound_timer_start, 0x18fc4, 0 },
  { (void *)sound_timer_start_body, 0x18fc6, 0 },
  { (void *)sound_vector, 0x1906a, 0 },
  { (void *)sound_timer_stop, 0x19084, 0 },
  { (void *)sound_off, 0x1909e, 0 },
  { (void *)sound_isr_load, 0x190b8, 0 },
  { (void *)sound_isr_back, 0x190f6, 0 },
  { (void *)sound_isr_fwd, 0x1910e, 0 },
  { (void *)ym_reset, 0x19282, 0 },
  { (void *)ym_write, 0x1929a, 0 },
  { (void *)noop_19312, 0x19312, 0 },
  { (void *)init_hardware, 0x3a556, 0 },
  { (void *)music_cmd_table, 0x3a57c, 0 },
  { (void *)music_install, 0x3a5c2, 0 },
  { (void *)mfp_setup, 0x3a604, 0 },
  { (void *)music_load, 0x3a62e, 0 },
  { (void *)music_stop, 0x3a6f4, 0 },
  { (void *)music_stop2, 0x3a712, 0 },
  { (void *)music_cmd, 0x3a734, 0 },
  { (void *)music_silence, 0x3a85e, 0 },
  { (void *)music_note, 0x3a992, 0 },
  { (void *)psg_trap3, 0x3aa38, 0 },
  { (void *)psg_write_verify, 0x3aa7c, 0 },
  { (void *)psg_shadow, 0x3aa8a, 0 },
  { (void *)music_tick, 0x3aab6, 0 },
  { (void *)music_h1, 0x3ad5a, 0 },
  { (void *)music_h2, 0x3ad6a, 0 },
  { (void *)music_h3, 0x3adb4, 0 },
  { (void *)music_h4, 0x3ae00, 0 },
  { (void *)music_h5, 0x3ae68, 0 },
};
static const unsigned k_cpuwork_nmarks = 413;
