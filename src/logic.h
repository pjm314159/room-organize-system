#ifndef LOGIC_H
#define LOGIC_H

#include "types.h"
#include "db.h"

int logic_add_room(void);
int logic_list_rooms(void);
int logic_update_room(void);
int logic_delete_room(void);

int logic_add_teacher(void);
int logic_list_teachers(void);
int logic_update_teacher(void);
int logic_delete_teacher(void);

int logic_add_course(void);
int logic_list_courses(void);
int logic_update_course(void);
int logic_delete_course(void);

int logic_add_schedule(void);
int logic_list_schedules(void);
int logic_query_schedules(void);
int logic_update_schedule(void);
int logic_delete_schedule(void);

int logic_show_statistics(void);

#endif
