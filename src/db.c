/**
 * @file db.c
 * @brief 数据库操作模块实现
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2024
 * 
 * @description
 * 本文件实现了所有数据库操作函数，使用 SQLite3 作为数据库引擎。
 * 包含数据库初始化、表创建、CRUD 操作、冲突检测等功能。
 */

#include "db.h"
#include "schedule_index.h"

/* 平台相关头文件 */
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#endif

/*============================================================================
 * 模块私有变量
 *============================================================================*/

/** @brief SQLite 数据库连接句柄（模块全局） */
static sqlite3 *db = NULL;

/** @brief 排课内存索引（模块全局），用于 O(1) 冲突检测 */
static ScheduleIndex *schedule_idx = NULL;

/*============================================================================
 * 内部辅助函数
 *============================================================================*/

/**
 * @brief 确保数据库文件所在目录存在
 * 
 * 从文件路径中提取目录路径，如果目录不存在则创建。
 * 支持跨平台（Windows/Linux）。
 * 
 * @param filepath 数据库文件完整路径
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
static int ensure_directory_exists(const char *filepath) {
    char dir_path[512];
    
    /* 查找最后一个路径分隔符 */
    const char *last_slash = strrchr(filepath, '/');
    const char *last_backslash = strrchr(filepath, '\\');
    const char *last_sep = last_slash > last_backslash ? last_slash : last_backslash;
    
    /* 如果没有路径分隔符，说明在当前目录 */
    if (!last_sep) {
        return SUCCESS;
    }
    
    /* 提取目录路径 */
    size_t dir_len = last_sep - filepath;
    if (dir_len >= sizeof(dir_path)) {
        return FAILURE;
    }
    
    strncpy(dir_path, filepath, dir_len);
    dir_path[dir_len] = '\0';
    
    /* 创建目录（平台相关） */
#ifdef _WIN32
    if (CreateDirectoryA(dir_path, NULL) == 0) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            return FAILURE;
        }
    }
#else
    if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
        return FAILURE;
    }
#endif
    
    return SUCCESS;
}

/*============================================================================
 * 数据库生命周期管理
 *============================================================================*/

int db_init(const char *db_path) {
    /* 确保数据目录存在 */
    if (ensure_directory_exists(db_path) != SUCCESS) {
        fprintf(stderr, "无法创建数据目录\n");
    }
    
    /* 打开数据库文件 */
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        return FAILURE;
    }

    /* 创建数据表（如果不存在） */
    const char *sql =
        /* 机房表 */
        "CREATE TABLE IF NOT EXISTS computer_room ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    room_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    capacity INTEGER NOT NULL,"
        "    status INTEGER DEFAULT 0"
        ");"

        /* 教师表 */
        "CREATE TABLE IF NOT EXISTS teacher ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    teacher_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    phone TEXT"
        ");"

        /* 课程表 */
        "CREATE TABLE IF NOT EXISTS course ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    course_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    teacher_id INTEGER,"
        "    hours INTEGER,"
        "    FOREIGN KEY (teacher_id) REFERENCES teacher(id)"
        ");"

        /* 排课表 */
        "CREATE TABLE IF NOT EXISTS schedule ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    room_id INTEGER NOT NULL,"
        "    course_id INTEGER NOT NULL,"
        "    teacher_id INTEGER NOT NULL,"
        "    day_of_week INTEGER NOT NULL,"
        "    period INTEGER NOT NULL,"
        "    FOREIGN KEY (room_id) REFERENCES computer_room(id),"
        "    FOREIGN KEY (course_id) REFERENCES course(id),"
        "    FOREIGN KEY (teacher_id) REFERENCES teacher(id)"
        ");";

    char *err_msg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return FAILURE;
    }

    /* 初始化内存索引，从数据库加载已有排课记录 */
    schedule_idx = schedule_index_create();
    if (schedule_idx) {
        schedule_index_load(schedule_idx);
    }

    return SUCCESS;
}

void db_close(void) {
    /* 销毁内存索引 */
    if (schedule_idx) {
        schedule_index_destroy(schedule_idx);
        schedule_idx = NULL;
    }

    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

/*============================================================================
 * 机房表操作 (computer_room)
 *============================================================================*/

int db_create_room(ComputerRoom *room) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO computer_room (room_no, name, capacity, status) VALUES ('%s', '%s', %d, %d);",
        room->room_no, room->name, room->capacity, room->status);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return ERROR_DUPLICATE;  /* 唯一键冲突 */
    }

    /* 获取自增ID */
    room->id = (int)sqlite3_last_insert_rowid(db);
    return SUCCESS;
}

int db_get_all_rooms(ComputerRoom **rooms, int *count) {
    const char *sql = "SELECT id, room_no, name, capacity, status FROM computer_room ORDER BY room_no;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *rooms = NULL;

    /* 遍历结果集 */
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *rooms = realloc(*rooms, (*count + 1) * sizeof(ComputerRoom));
        ComputerRoom *r = &(*rooms)[*count];
        r->id = sqlite3_column_int(stmt, 0);
        strcpy(r->room_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(r->name, (const char*)sqlite3_column_text(stmt, 2));
        r->capacity = sqlite3_column_int(stmt, 3);
        r->status = sqlite3_column_int(stmt, 4);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

int db_get_room_by_id(int id, ComputerRoom *room) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_no, name, capacity, status FROM computer_room WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        room->id = sqlite3_column_int(stmt, 0);
        strcpy(room->room_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(room->name, (const char*)sqlite3_column_text(stmt, 2));
        room->capacity = sqlite3_column_int(stmt, 3);
        room->status = sqlite3_column_int(stmt, 4);
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

int db_update_room(ComputerRoom *room) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE computer_room SET room_no='%s', name='%s', capacity=%d, status=%d WHERE id=%d;",
        room->room_no, room->name, room->capacity, room->status, room->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

int db_delete_room(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM computer_room WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

/*============================================================================
 * 教师表操作 (teacher)
 *============================================================================*/

int db_create_teacher(Teacher *teacher) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO teacher (teacher_no, name, phone) VALUES ('%s', '%s', '%s');",
        teacher->teacher_no, teacher->name, teacher->phone);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return ERROR_DUPLICATE;
    }

    teacher->id = (int)sqlite3_last_insert_rowid(db);
    return SUCCESS;
}

int db_get_all_teachers(Teacher **teachers, int *count) {
    const char *sql = "SELECT id, teacher_no, name, phone FROM teacher ORDER BY teacher_no;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *teachers = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *teachers = realloc(*teachers, (*count + 1) * sizeof(Teacher));
        Teacher *t = &(*teachers)[*count];
        t->id = sqlite3_column_int(stmt, 0);
        strcpy(t->teacher_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(t->name, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(t->phone, (const char*)sqlite3_column_text(stmt, 3));
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

int db_get_teacher_by_id(int id, Teacher *teacher) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, teacher_no, name, phone FROM teacher WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        teacher->id = sqlite3_column_int(stmt, 0);
        strcpy(teacher->teacher_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(teacher->name, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(teacher->phone, (const char*)sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

int db_update_teacher(Teacher *teacher) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE teacher SET teacher_no='%s', name='%s', phone='%s' WHERE id=%d;",
        teacher->teacher_no, teacher->name, teacher->phone, teacher->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

int db_delete_teacher(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM teacher WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

/*============================================================================
 * 课程表操作 (course)
 *============================================================================*/

int db_create_course(Course *course) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO course (course_no, name, teacher_id, hours) VALUES ('%s', '%s', %d, %d);",
        course->course_no, course->name, course->teacher_id, course->hours);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return ERROR_DUPLICATE;
    }

    course->id = (int)sqlite3_last_insert_rowid(db);
    return SUCCESS;
}

int db_get_all_courses(Course **courses, int *count) {
    const char *sql = "SELECT id, course_no, name, teacher_id, hours FROM course ORDER BY course_no;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *courses = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *courses = realloc(*courses, (*count + 1) * sizeof(Course));
        Course *c = &(*courses)[*count];
        c->id = sqlite3_column_int(stmt, 0);
        strcpy(c->course_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(c->name, (const char*)sqlite3_column_text(stmt, 2));
        c->teacher_id = sqlite3_column_int(stmt, 3);
        c->hours = sqlite3_column_int(stmt, 4);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

int db_get_course_by_id(int id, Course *course) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, course_no, name, teacher_id, hours FROM course WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        course->id = sqlite3_column_int(stmt, 0);
        strcpy(course->course_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(course->name, (const char*)sqlite3_column_text(stmt, 2));
        course->teacher_id = sqlite3_column_int(stmt, 3);
        course->hours = sqlite3_column_int(stmt, 4);
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

int db_update_course(Course *course) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE course SET course_no='%s', name='%s', teacher_id=%d, hours=%d WHERE id=%d;",
        course->course_no, course->name, course->teacher_id, course->hours, course->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

int db_delete_course(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM course WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

/*============================================================================
 * 排课表操作 (schedule)
 *============================================================================*/

int db_create_schedule(Schedule *schedule) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO schedule (room_id, course_id, teacher_id, day_of_week, period) VALUES (%d, %d, %d, %d, %d);",
        schedule->room_id, schedule->course_id, schedule->teacher_id,
        schedule->day_of_week, schedule->period);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    schedule->id = (int)sqlite3_last_insert_rowid(db);

    /* 同步更新内存索引 */
    if (schedule_idx) {
        schedule_index_add(schedule_idx, schedule);
    }

    return SUCCESS;
}

int db_get_all_schedules(Schedule **schedules, int *count) {
    const char *sql = "SELECT id, room_id, course_id, teacher_id, day_of_week, period FROM schedule ORDER BY day_of_week, period;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

int db_get_schedule_by_id(int id, Schedule *schedule) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period FROM schedule WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        schedule->id = sqlite3_column_int(stmt, 0);
        schedule->room_id = sqlite3_column_int(stmt, 1);
        schedule->course_id = sqlite3_column_int(stmt, 2);
        schedule->teacher_id = sqlite3_column_int(stmt, 3);
        schedule->day_of_week = sqlite3_column_int(stmt, 4);
        schedule->period = sqlite3_column_int(stmt, 5);
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

int db_update_schedule(Schedule *schedule) {
    /* 先获取旧记录用于索引更新 */
    Schedule old_schedule;
    int has_old = (db_get_schedule_by_id(schedule->id, &old_schedule) == SUCCESS);

    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE schedule SET room_id=%d, course_id=%d, teacher_id=%d, day_of_week=%d, period=%d WHERE id=%d;",
        schedule->room_id, schedule->course_id, schedule->teacher_id,
        schedule->day_of_week, schedule->period, schedule->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    int result = sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;

    /* 同步更新内存索引 */
    if (result == SUCCESS && schedule_idx && has_old) {
        schedule_index_update(schedule_idx, &old_schedule, schedule);
    }

    return result;
}

int db_delete_schedule(int id) {
    /* 先获取旧记录用于索引更新 */
    Schedule old_schedule;
    int has_old = (db_get_schedule_by_id(id, &old_schedule) == SUCCESS);

    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM schedule WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    int result = sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;

    /* 同步更新内存索引 */
    if (result == SUCCESS && schedule_idx && has_old) {
        schedule_index_remove(schedule_idx, &old_schedule);
    }

    return result;
}

/*============================================================================
 * 排课详情查询
 *============================================================================*/

int db_get_schedule_details(ScheduleDetail **details, int *count) {
    /* 使用 JOIN 查询关联信息 */
    const char *sql =
        "SELECT s.id, s.room_id, r.name, s.course_id, c.name, "
        "       s.teacher_id, t.name, s.day_of_week, s.period "
        "FROM schedule s "
        "JOIN computer_room r ON s.room_id = r.id "
        "JOIN course c ON s.course_id = c.id "
        "JOIN teacher t ON s.teacher_id = t.id "
        "ORDER BY s.day_of_week, s.period;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *details = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *details = realloc(*details, (*count + 1) * sizeof(ScheduleDetail));
        ScheduleDetail *d = &(*details)[*count];
        d->schedule_id = sqlite3_column_int(stmt, 0);
        d->room_id = sqlite3_column_int(stmt, 1);
        strcpy(d->room_name, (const char*)sqlite3_column_text(stmt, 2));
        d->course_id = sqlite3_column_int(stmt, 3);
        strcpy(d->course_name, (const char*)sqlite3_column_text(stmt, 4));
        d->teacher_id = sqlite3_column_int(stmt, 5);
        strcpy(d->teacher_name, (const char*)sqlite3_column_text(stmt, 6));
        d->day_of_week = sqlite3_column_int(stmt, 7);
        d->period = sqlite3_column_int(stmt, 8);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

/*============================================================================
 * 条件查询
 *============================================================================*/

int db_get_schedules_by_room(int room_id, Schedule **schedules, int *count) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period "
        "FROM schedule WHERE room_id = %d ORDER BY day_of_week, period;", room_id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

int db_get_schedules_by_teacher(int teacher_id, Schedule **schedules, int *count) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period "
        "FROM schedule WHERE teacher_id = %d ORDER BY day_of_week, period;", teacher_id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

int db_get_schedules_by_day(int day_of_week, Schedule **schedules, int *count) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period "
        "FROM schedule WHERE day_of_week = %d ORDER BY period;", day_of_week);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

/*============================================================================
 * 冲突检测
 *============================================================================*/

int db_check_room_conflict(int room_id, int day_of_week, int period) {
    /* 优先使用内存索引 O(1)，回退到数据库查询 O(n) */
    if (schedule_idx) {
        return schedule_index_check_room_conflict(schedule_idx, room_id, day_of_week, period);
    }

    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT COUNT(*) FROM schedule WHERE room_id = %d AND day_of_week = %d AND period = %d;",
        room_id, day_of_week, period);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0 ? ERROR_CONFLICT : SUCCESS;
}

int db_check_teacher_conflict(int teacher_id, int day_of_week, int period) {
    /* 优先使用内存索引 O(1)，回退到数据库查询 O(n) */
    if (schedule_idx) {
        return schedule_index_check_teacher_conflict(schedule_idx, teacher_id, day_of_week, period);
    }

    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT COUNT(*) FROM schedule WHERE teacher_id = %d AND day_of_week = %d AND period = %d;",
        teacher_id, day_of_week, period);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0 ? ERROR_CONFLICT : SUCCESS;
}

/*============================================================================
 * 统计查询
 *============================================================================*/

int db_get_room_usage_count(int room_id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT COUNT(*) FROM schedule WHERE room_id = %d;", room_id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}
