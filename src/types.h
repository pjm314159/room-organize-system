#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DB_PATH "./data/schedule.db"
#define MAX_SQL_LENGTH 1024

#define DAYS_PER_WEEK 7
#define PERIODS_PER_DAY 8

#define SUCCESS 1
#define FAILURE 0
#define ERROR_DUPLICATE -1
#define ERROR_NOT_FOUND -2
#define ERROR_CONFLICT -3
#define ERROR_INVALID_INPUT -4

typedef enum {
    ROOM_STATUS_NORMAL = 0,
    ROOM_STATUS_MAINTENANCE = 1
} RoomStatus;

typedef struct {
    int id;
    char room_no[20];
    char name[50];
    int capacity;
    RoomStatus status;
} ComputerRoom;

typedef struct {
    int id;
    char teacher_no[20];
    char name[50];
    char phone[20];
} Teacher;

typedef struct {
    int id;
    char course_no[20];
    char name[50];
    int teacher_id;
    int hours;
} Course;

typedef struct {
    int id;
    int room_id;
    int course_id;
    int teacher_id;
    int day_of_week;
    int period;
} Schedule;

typedef struct {
    int schedule_id;
    int room_id;
    char room_name[50];
    int course_id;
    char course_name[50];
    int teacher_id;
    char teacher_name[50];
    int day_of_week;
    int period;
} ScheduleDetail;

typedef struct {
    int schedule_id;
    int course_id;
    int teacher_id;
} TimeSlot;

#endif
