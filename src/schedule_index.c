/**
 * @file schedule_index.c
 * @brief 排课内存索引模块实现
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2026
 * 
 * @description
 * 本文件实现了排课内存索引，包括：
 * 1. 哈希表索引 - 冲突检测 O(n) → O(1)
 * 2. 二维数组索引 - 时间槽直接寻址 O(1)
 * 
 * 性能对比：
 * | 操作         | 优化前  | 优化后  |
 * |-------------|---------|---------|
 * | 冲突检测     | O(n)    | O(1)    |
 * | 查询某时段   | O(n)    | O(1)    |
 * | 统计利用率   | O(n)    | O(1)    |
 */

#include "schedule_index.h"
#include "db.h"

/*============================================================================
 * 内部辅助函数
 *============================================================================*/

/**
 * @brief DJB2 哈希函数
 * 
 * 经典字符串哈希算法，分布均匀，冲突率低。
 * 
 * @param str 输入字符串
 * @return 哈希值
 */
static unsigned int hash_djb2(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/**
 * @brief 生成机房时间槽键
 * 
 * 格式："room_id-day_of_week-period"
 * 例如："1-3-2" 表示机房1-周三-第2节
 * 
 * @param room_id 机房ID
 * @param day_of_week 星期几
 * @param period 课时
 * @param buf 输出缓冲区（至少 32 字节）
 */
static void make_room_time_key(int room_id, int day_of_week, int period, char *buf) {
    sprintf(buf, "r%d-%d-%d", room_id, day_of_week, period);
}

/**
 * @brief 生成教师时间槽键
 * 
 * 格式："teacher_id-day_of_week-period"
 * 例如："2-1-4" 表示教师2-周一-第4节
 * 
 * @param teacher_id 教师ID
 * @param day_of_week 星期几
 * @param period 课时
 * @param buf 输出缓冲区（至少 32 字节）
 */
static void make_teacher_time_key(int teacher_id, int day_of_week, int period, char *buf) {
    sprintf(buf, "t%d-%d-%d", teacher_id, day_of_week, period);
}

/*============================================================================
 * 哈希表操作实现
 *============================================================================*/

HashTable* hash_table_create(int capacity) {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table) return NULL;

    table->buckets = calloc(capacity, sizeof(HashNode*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }

    table->capacity = capacity;
    table->count = 0;
    return table;
}

void hash_table_destroy(HashTable *table) {
    if (!table) return;

    /* 遍历每个桶，释放链表节点 */
    for (int i = 0; i < table->capacity; i++) {
        HashNode *node = table->buckets[i];
        while (node) {
            HashNode *next = node->next;
            free(node);
            node = next;
        }
    }

    free(table->buckets);
    free(table);
}

int hash_table_set(HashTable *table, const char *key, int value) {
    if (!table || !key) return FAILURE;

    unsigned int index = hash_djb2(key) % table->capacity;

    /* 检查是否已存在，若存在则更新值 */
    HashNode *node = table->buckets[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            node->value = value;
            return SUCCESS;
        }
        node = node->next;
    }

    /* 创建新节点，头插法 */
    HashNode *new_node = malloc(sizeof(HashNode));
    if (!new_node) return FAILURE;

    strncpy(new_node->key, key, sizeof(new_node->key) - 1);
    new_node->key[sizeof(new_node->key) - 1] = '\0';
    new_node->value = value;
    new_node->next = table->buckets[index];
    table->buckets[index] = new_node;
    table->count++;

    return SUCCESS;
}

int hash_table_get(HashTable *table, const char *key) {
    if (!table || !key) return -1;

    unsigned int index = hash_djb2(key) % table->capacity;

    HashNode *node = table->buckets[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }

    return -1;
}

int hash_table_remove(HashTable *table, const char *key) {
    if (!table || !key) return ERROR_NOT_FOUND;

    unsigned int index = hash_djb2(key) % table->capacity;

    HashNode *prev = NULL;
    HashNode *node = table->buckets[index];

    while (node) {
        if (strcmp(node->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                table->buckets[index] = node->next;
            }
            free(node);
            table->count--;
            return SUCCESS;
        }
        prev = node;
        node = node->next;
    }

    return ERROR_NOT_FOUND;
}

/*============================================================================
 * 排课索引管理实现
 *============================================================================*/

ScheduleIndex* schedule_index_create(void) {
    ScheduleIndex *index = malloc(sizeof(ScheduleIndex));
    if (!index) return NULL;

    /* 创建机房时间哈希索引 */
    index->room_time_index = hash_table_create(HASH_TABLE_DEFAULT_CAPACITY);
    if (!index->room_time_index) {
        free(index);
        return NULL;
    }

    /* 创建教师时间哈希索引 */
    index->teacher_time_index = hash_table_create(HASH_TABLE_DEFAULT_CAPACITY);
    if (!index->teacher_time_index) {
        hash_table_destroy(index->room_time_index);
        free(index);
        return NULL;
    }

    /* 初始化机房周时间表数组 */
    index->room_schedules = NULL;
    index->room_schedule_count = 0;
    index->room_schedule_capacity = 0;

    return index;
}

void schedule_index_destroy(ScheduleIndex *index) {
    if (!index) return;

    hash_table_destroy(index->room_time_index);
    hash_table_destroy(index->teacher_time_index);
    free(index->room_schedules);
    free(index);
}

int schedule_index_load(ScheduleIndex *index) {
    if (!index) return FAILURE;

    /* 从数据库加载所有排课记录 */
    Schedule *schedules = NULL;
    int count = 0;

    if (db_get_all_schedules(&schedules, &count) != SUCCESS) {
        return FAILURE;
    }

    /* 从数据库加载所有机房，初始化二维数组 */
    ComputerRoom *rooms = NULL;
    int room_count = 0;
    db_get_all_rooms(&rooms, &room_count);

    if (room_count > 0) {
        index->room_schedule_capacity = room_count + 10;
        index->room_schedules = realloc(index->room_schedules,
            index->room_schedule_capacity * sizeof(RoomWeekSchedule));
        index->room_schedule_count = room_count;

        /* 初始化每个机房的时间表 */
        for (int i = 0; i < room_count; i++) {
            index->room_schedules[i].room_id = rooms[i].id;
            strcpy(index->room_schedules[i].room_name, rooms[i].name);
            for (int d = 0; d < DAYS_PER_WEEK; d++) {
                for (int p = 0; p < PERIODS_PER_DAY; p++) {
                    index->room_schedules[i].schedule_table[d][p].schedule_id = -1;
                    index->room_schedules[i].schedule_table[d][p].course_id = -1;
                    index->room_schedules[i].schedule_table[d][p].teacher_id = -1;
                }
            }
        }
    }
    free(rooms);

    /* 将排课记录添加到索引 */
    for (int i = 0; i < count; i++) {
        schedule_index_add(index, &schedules[i]);
    }

    free(schedules);
    return SUCCESS;
}

int schedule_index_add(ScheduleIndex *index, Schedule *schedule) {
    if (!index || !schedule) return FAILURE;

    char key[32];

    /* 添加到机房时间哈希索引 */
    make_room_time_key(schedule->room_id, schedule->day_of_week, schedule->period, key);
    hash_table_set(index->room_time_index, key, schedule->id);

    /* 添加到教师时间哈希索引 */
    make_teacher_time_key(schedule->teacher_id, schedule->day_of_week, schedule->period, key);
    hash_table_set(index->teacher_time_index, key, schedule->id);

    /* 添加到二维数组 */
    RoomWeekSchedule *ws = schedule_index_get_room_schedule(index, schedule->room_id);
    if (ws) {
        int d = schedule->day_of_week - 1;
        int p = schedule->period - 1;
        if (d >= 0 && d < DAYS_PER_WEEK && p >= 0 && p < PERIODS_PER_DAY) {
            ws->schedule_table[d][p].schedule_id = schedule->id;
            ws->schedule_table[d][p].course_id = schedule->course_id;
            ws->schedule_table[d][p].teacher_id = schedule->teacher_id;
        }
    }

    return SUCCESS;
}

int schedule_index_remove(ScheduleIndex *index, Schedule *schedule) {
    if (!index || !schedule) return FAILURE;

    char key[32];

    /* 从机房时间哈希索引删除 */
    make_room_time_key(schedule->room_id, schedule->day_of_week, schedule->period, key);
    hash_table_remove(index->room_time_index, key);

    /* 从教师时间哈希索引删除 */
    make_teacher_time_key(schedule->teacher_id, schedule->day_of_week, schedule->period, key);
    hash_table_remove(index->teacher_time_index, key);

    /* 从二维数组清除 */
    RoomWeekSchedule *ws = schedule_index_get_room_schedule(index, schedule->room_id);
    if (ws) {
        int d = schedule->day_of_week - 1;
        int p = schedule->period - 1;
        if (d >= 0 && d < DAYS_PER_WEEK && p >= 0 && p < PERIODS_PER_DAY) {
            ws->schedule_table[d][p].schedule_id = -1;
            ws->schedule_table[d][p].course_id = -1;
            ws->schedule_table[d][p].teacher_id = -1;
        }
    }

    return SUCCESS;
}

int schedule_index_update(ScheduleIndex *index, Schedule *old_schedule, Schedule *new_schedule) {
    if (!index || !old_schedule || !new_schedule) return FAILURE;

    /* 先删除旧索引 */
    schedule_index_remove(index, old_schedule);

    /* 检测新时间是否冲突 */
    if (schedule_index_check_room_conflict(index, new_schedule->room_id,
            new_schedule->day_of_week, new_schedule->period) == ERROR_CONFLICT) {
        /* 冲突时恢复旧索引 */
        schedule_index_add(index, old_schedule);
        return ERROR_CONFLICT;
    }

    if (schedule_index_check_teacher_conflict(index, new_schedule->teacher_id,
            new_schedule->day_of_week, new_schedule->period) == ERROR_CONFLICT) {
        /* 冲突时恢复旧索引 */
        schedule_index_add(index, old_schedule);
        return ERROR_CONFLICT;
    }

    /* 添加新索引 */
    schedule_index_add(index, new_schedule);

    return SUCCESS;
}

/*============================================================================
 * 快速冲突检测实现（O(1)）
 *============================================================================*/

int schedule_index_check_room_conflict(ScheduleIndex *index, int room_id, int day_of_week, int period) {
    if (!index) return FAILURE;

    char key[32];
    make_room_time_key(room_id, day_of_week, period, key);

    /* 哈希表查找，O(1) 平均时间复杂度 */
    int result = hash_table_get(index->room_time_index, key);
    return result != -1 ? ERROR_CONFLICT : SUCCESS;
}

int schedule_index_check_teacher_conflict(ScheduleIndex *index, int teacher_id, int day_of_week, int period) {
    if (!index) return FAILURE;

    char key[32];
    make_teacher_time_key(teacher_id, day_of_week, period, key);

    /* 哈希表查找，O(1) 平均时间复杂度 */
    int result = hash_table_get(index->teacher_time_index, key);
    return result != -1 ? ERROR_CONFLICT : SUCCESS;
}

/*============================================================================
 * 二维数组快速查询实现
 *============================================================================*/

RoomWeekSchedule* schedule_index_get_room_schedule(ScheduleIndex *index, int room_id) {
    if (!index || !index->room_schedules) return NULL;

    /* 线性查找机房（机房数量通常较少，性能可接受） */
    for (int i = 0; i < index->room_schedule_count; i++) {
        if (index->room_schedules[i].room_id == room_id) {
            return &index->room_schedules[i];
        }
    }

    return NULL;
}

float schedule_index_get_room_usage_rate(ScheduleIndex *index, int room_id) {
    if (!index) return 0.0f;

    RoomWeekSchedule *ws = schedule_index_get_room_schedule(index, room_id);
    if (!ws) return 0.0f;

    int total_slots = DAYS_PER_WEEK * PERIODS_PER_DAY;
    int used_slots = 0;

    /* 遍历二维数组统计已用时间槽 */
    for (int d = 0; d < DAYS_PER_WEEK; d++) {
        for (int p = 0; p < PERIODS_PER_DAY; p++) {
            if (ws->schedule_table[d][p].schedule_id != -1) {
                used_slots++;
            }
        }
    }

    return (float)used_slots / total_slots * 100.0f;
}

int schedule_index_get_day_count(ScheduleIndex *index, int room_id, int day_of_week) {
    if (!index) return 0;

    RoomWeekSchedule *ws = schedule_index_get_room_schedule(index, room_id);
    if (!ws) return 0;

    int d = day_of_week - 1;
    if (d < 0 || d >= DAYS_PER_WEEK) return 0;

    int count = 0;
    for (int p = 0; p < PERIODS_PER_DAY; p++) {
        if (ws->schedule_table[d][p].schedule_id != -1) {
            count++;
        }
    }

    return count;
}
