#include "db.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#endif

static sqlite3 *db = NULL;

static int ensure_directory_exists(const char *filepath) {
    char dir_path[512];
    const char *last_slash = strrchr(filepath, '/');
    const char *last_backslash = strrchr(filepath, '\\');
    const char *last_sep = last_slash > last_backslash ? last_slash : last_backslash;
    
    if (!last_sep) {
        return SUCCESS;
    }
    
    size_t dir_len = last_sep - filepath;
    if (dir_len >= sizeof(dir_path)) {
        return FAILURE;
    }
    
    strncpy(dir_path, filepath, dir_len);
    dir_path[dir_len] = '\0';
    
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

int db_init(const char *db_path) {
    if (ensure_directory_exists(db_path) != SUCCESS) {
        fprintf(stderr, "无法创建数据目录\n");
    }
    
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        return FAILURE;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS computer_room ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    room_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    capacity INTEGER NOT NULL,"
        "    status INTEGER DEFAULT 0"
        ");"

        "CREATE TABLE IF NOT EXISTS teacher ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    teacher_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    phone TEXT"
        ");"

        "CREATE TABLE IF NOT EXISTS course ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    course_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    teacher_id INTEGER,"
        "    hours INTEGER,"
        "    FOREIGN KEY (teacher_id) REFERENCES teacher(id)"
        ");"

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

    return SUCCESS;
}

void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

int db_create_room(ComputerRoom *room) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO computer_room (room_no, name, capacity, status) VALUES ('%s', '%s', %d, %d);",
        room->room_no, room->name, room->capacity, room->status);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return ERROR_DUPLICATE;
    }

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
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE schedule SET room_id=%d, course_id=%d, teacher_id=%d, day_of_week=%d, period=%d WHERE id=%d;",
        schedule->room_id, schedule->course_id, schedule->teacher_id,
        schedule->day_of_week, schedule->period, schedule->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

int db_delete_schedule(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM schedule WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

int db_get_schedule_details(ScheduleDetail **details, int *count) {
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

int db_check_room_conflict(int room_id, int day_of_week, int period) {
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
