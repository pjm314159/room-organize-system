#ifndef DB_H
#define DB_H

#include "types.h"
#include <sqlite3.h>

int db_init(const char *db_path);
void db_close(void);

int db_create_room(ComputerRoom *room);
int db_get_all_rooms(ComputerRoom **rooms, int *count);
int db_get_room_by_id(int id, ComputerRoom *room);
int db_update_room(ComputerRoom *room);
int db_delete_room(int id);

int db_create_teacher(Teacher *teacher);
int db_get_all_teachers(Teacher **teachers, int *count);
int db_get_teacher_by_id(int id, Teacher *teacher);
int db_update_teacher(Teacher *teacher);
int db_delete_teacher(int id);

int db_create_course(Course *course);
int db_get_all_courses(Course **courses, int *count);
int db_get_course_by_id(int id, Course *course);
int db_update_course(Course *course);
int db_delete_course(int id);

int db_create_schedule(Schedule *schedule);
int db_get_all_schedules(Schedule **schedules, int *count);
int db_get_schedule_by_id(int id, Schedule *schedule);
int db_update_schedule(Schedule *schedule);
int db_delete_schedule(int id);

int db_get_schedule_details(ScheduleDetail **details, int *count);

int db_get_schedules_by_room(int room_id, Schedule **schedules, int *count);
int db_get_schedules_by_teacher(int teacher_id, Schedule **schedules, int *count);
int db_get_schedules_by_day(int day_of_week, Schedule **schedules, int *count);

int db_check_room_conflict(int room_id, int day_of_week, int period);
int db_check_teacher_conflict(int teacher_id, int day_of_week, int period);

int db_get_room_usage_count(int room_id);

#endif
