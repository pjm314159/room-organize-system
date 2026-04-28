#ifndef UI_H
#define UI_H

#include "types.h"

void ui_clear_screen(void);
void ui_clear_line(void);

void ui_show_main_menu(void);
int ui_get_main_choice(void);

void ui_show_data_menu(void);
int ui_get_data_choice(void);

void ui_show_room_menu(void);
int ui_get_room_choice(void);

void ui_show_course_menu(void);
int ui_get_course_choice(void);

void ui_show_teacher_menu(void);
int ui_get_teacher_choice(void);

void ui_show_schedule_menu(void);
int ui_get_schedule_choice(void);

void ui_input_string(const char *prompt, char *buffer, int max_len);
int ui_input_int(const char *prompt);
int ui_input_int_range(const char *prompt, int min, int max);
char ui_input_confirm(const char *prompt);

void ui_show_title(const char *title);
void ui_show_divider(void);
void ui_show_success(const char *message);
void ui_show_error(const char *message);
void ui_show_warning(const char *message);
void ui_pause(void);

void ui_display_room(ComputerRoom *room);
void ui_display_rooms(ComputerRoom *rooms, int count);

void ui_display_teacher(Teacher *teacher);
void ui_display_teachers(Teacher *teachers, int count);

void ui_display_course(Course *course);
void ui_display_courses(Course *courses, int count);

void ui_display_schedule(Schedule *schedule);
void ui_display_schedule_detail(ScheduleDetail *detail);
void ui_display_schedules(Schedule *schedules, int count);
void ui_display_schedule_details(ScheduleDetail *details, int count);

const char* ui_get_day_name(int day);

#endif
