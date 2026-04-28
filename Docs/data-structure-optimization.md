# 数据结构优化方案

## 1. 时间冲突检测 - 哈希表

### 1.1 问题分析

每次创建排课时，需要遍历所有排课记录来检查冲突，时间复杂度 O(n)。当数据量增大时，查询效率会明显下降。

### 1.2 优化方案

使用哈希表，以 "机房ID-星期-课时" 和 "教师ID-星期-课时" 作为键建立索引。

### 1.3 数据结构定义

```c
// 时间槽定义
typedef struct {
    int room_id;
    int day;
    int period;
    int schedule_id;
} TimeSlot;

// 哈希表节点
typedef struct HashNode {
    char *key;
    void *value;
    struct HashNode *next;
} HashNode;

// 哈希表
typedef struct {
    HashNode **buckets;
    int size;
    int count;
} HashTable;

// 排课管理器
typedef struct {
    HashTable *room_time_index;      // 机房时间索引
    HashTable *teacher_time_index;   // 教师时间索引
} ScheduleIndex;
```

### 1.4 键的生成

```c
// 生成机房时间槽键
char* make_room_time_key(int room_id, int day, int period) {
    char *key = malloc(32);
    sprintf(key, "%d-%d-%d", room_id, day, period);
    return key;
}

// 生成教师时间槽键
char* make_teacher_time_key(int teacher_id, int day, int period) {
    char *key = malloc(32);
    sprintf(key, "%d-%d-%d", teacher_id, day, period);
    return key;
}
```

### 1.5 核心操作

```c
// 哈希函数
unsigned int hash(const char *key, int capacity) {
    unsigned int hash = 0;
    while (*key) {
        hash = hash * 31 + *key++;
    }
    return hash % capacity;
}

// 冲突检测 - 机房
int check_room_conflict(ScheduleIndex *index, int room_id, int day, int period) {
    char *key = make_room_time_key(room_id, day, period);
    int exists = hash_table_get(index->room_time_index, key) != NULL;
    free(key);
    return exists;
}

// 冲突检测 - 教师
int check_teacher_conflict(ScheduleIndex *index, int teacher_id, int day, int period) {
    char *key = make_teacher_time_key(teacher_id, day, period);
    int exists = hash_table_get(index->teacher_time_index, key) != NULL;
    free(key);
    return exists;
}

// 添加排课到索引
int add_to_index(ScheduleIndex *index, Schedule *s) {
    // 机房冲突检测
    if (check_room_conflict(index, s->room_id, s->day, s->period)) {
        return 0;  // 冲突
    }

    // 教师冲突检测
    if (check_teacher_conflict(index, s->teacher_id, s->day, s->period)) {
        return 0;  // 冲突
    }

    // 添加到索引
    char *room_key = make_room_time_key(s->room_id, s->day, s->period);
    char *teacher_key = make_teacher_time_key(s->teacher_id, s->day, s->period);

    hash_table_set(index->room_time_index, room_key, s);
    hash_table_set(index->teacher_time_index, teacher_key, s);

    free(room_key);
    free(teacher_key);
    return 1;
}

// 从索引删除排课
void remove_from_index(ScheduleIndex *index, Schedule *s) {
    char *room_key = make_room_time_key(s->room_id, s->day, s->period);
    char *teacher_key = make_teacher_time_key(s->teacher_id, s->day, s->period);

    hash_table_delete(index->room_time_index, room_key);
    hash_table_delete(index->teacher_time_index, teacher_key);

    free(room_key);
    free(teacher_key);
}
```

### 1.6 性能对比

| 操作 | 优化前 | 优化后 |
|------|--------|--------|
| 冲突检测 | O(n) | O(1) |
| 添加排课 | O(n) | O(1) |
| 删除排课 | O(n) | O(1) |

---

## 2. 时间范围查询 - 平衡树（AVL树）

### 2.1 问题分析

按星期查询所有排课，需要扫描整个排课表。如果需要查询某个时间段内的所有排课，效率较低。

### 2.2 优化方案

使用 AVL 平衡树，按 (day_of_week, period) 建立索引，支持高效的范围查询。

### 2.3 数据结构定义

```c
// 排课节点
typedef struct ScheduleNode {
    Schedule data;
    struct ScheduleNode *left;
    struct ScheduleNode *right;
    int height;
    int balance_factor;
} ScheduleNode;

// 比较函数
int compare_schedule(Schedule *a, Schedule *b) {
    if (a->day != b->day) {
        return a->day - b->day;
    }
    return a->period - b->period;
}

// 辅助函数
int get_height(ScheduleNode *node) {
    return node ? node->height : 0;
}

int get_balance(ScheduleNode *node) {
    return node ? get_height(node->left) - get_height(node->right) : 0;
}
```

### 2.4 AVL 树操作

```c
// 右旋
ScheduleNode* rotate_right(ScheduleNode *y) {
    ScheduleNode *x = y->left;
    ScheduleNode *T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = 1 + max(get_height(y->left), get_height(y->right));
    x->height = 1 + max(get_height(x->left), get_height(x->right));

    return x;
}

// 左旋
ScheduleNode* rotate_left(ScheduleNode *x) {
    ScheduleNode *y = x->right;
    ScheduleNode *T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = 1 + max(get_height(x->left), get_height(x->right));
    y->height = 1 + max(get_height(y->left), get_height(y->right));

    return y;
}

// 插入节点
ScheduleNode* insert_schedule(ScheduleNode *node, Schedule *data) {
    if (!node) {
        ScheduleNode *new_node = malloc(sizeof(ScheduleNode));
        new_node->data = *data;
        new_node->left = NULL;
        new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }

    int cmp = compare_schedule(&node->data, data);
    if (cmp > 0) {
        node->left = insert_schedule(node->left, data);
    } else if (cmp < 0) {
        node->right = insert_schedule(node->right, data);
    } else {
        return node;  // 已存在，不重复插入
    }

    node->height = 1 + max(get_height(node->left), get_height(node->right));
    int balance = get_balance(node);

    // 左左情况
    if (balance > 1 && compare_schedule(&node->left->data, data) > 0) {
        return rotate_right(node);
    }
    // 右右情况
    if (balance < -1 && compare_schedule(&node->right->data, data) < 0) {
        return rotate_left(node);
    }
    // 左右情况
    if (balance > 1 && compare_schedule(&node->left->data, data) < 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    // 右左情况
    if (balance < -1 && compare_schedule(&node->right->data, data) > 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}
```

### 2.5 范围查询

```c
// 查询某天的所有排课（按课时排序）
void query_by_day(ScheduleNode *node, int day, Schedule **results, int *count) {
    if (!node) return;

    if (node->data.day_of_week == day) {
        results[*count] = &node->data;
        (*count)++;
        query_by_day(node->left, day, results, count);
        query_by_day(node->right, day, results, count);
    } else if (node->data.day_of_week > day) {
        query_by_day(node->left, day, results, count);
    } else {
        query_by_day(node->right, day, results, count);
    }
}

// 查询某个时间段的所有排课
void query_by_period_range(ScheduleNode *node, int day, int period_start,
                           int period_end, Schedule **results, int *count) {
    if (!node) return;

    if (node->data.day_of_week == day) {
        if (node->data.period >= period_start && node->data.period <= period_end) {
            results[*count] = &node->data;
            (*count)++;
        }
        if (period_start <= node->data.period) {
            query_by_period_range(node->left, day, period_start, period_end, results, count);
        }
        if (period_end >= node->data.period) {
            query_by_period_range(node->right, day, period_start, period_end, results, count);
        }
    } else if (node->data.day_of_week > day) {
        query_by_period_range(node->left, day, period_start, period_end, results, count);
    } else {
        query_by_period_range(node->right, day, period_start, period_end, results, count);
    }
}
```

### 2.6 性能对比

| 操作 | 优化前 | 优化后 |
|------|--------|--------|
| 按天查询 | O(n) | O(log n + k) |
| 范围查询 | O(n) | O(log n + k) |
| 插入 | O(1) | O(log n) |
| 删除 | O(n) | O(log n) |

---

## 3. 固定时间槽 - 二维数组

### 3.1 问题分析

机房使用情况统计需要遍历计算。如果需要频繁查看某个机房某时段的使用情况，用数组直接寻址最快速。

### 3.2 优化方案

用固定大小的二维数组表示每周的时间表，每个元素存储排课ID。

### 3.3 数据结构定义

```c
// 机房时间表
#define DAYS_PER_WEEK 7
#define PERIODS_PER_DAY 8

typedef struct {
    int schedule_id;      // -1 表示空闲
    int course_id;        // 课程ID
    int teacher_id;       // 教师ID
} TimeSlot;

typedef struct {
    int room_id;
    char name[50];
    TimeSlot schedule_table[DAYS_PER_WEEK][PERIODS_PER_DAY];
} ComputerRoomSchedule;

// 所有机房的时间表
typedef struct {
    ComputerRoomSchedule *rooms;
    int room_count;
} WeeklyScheduleMap;
```

### 3.4 核心操作

```c
// 初始化时间表
void init_room_schedule(ComputerRoomSchedule *room) {
    for (int day = 0; day < DAYS_PER_WEEK; day++) {
        for (int period = 0; period < PERIODS_PER_DAY; period++) {
            room->schedule_table[day][period].schedule_id = -1;
        }
    }
}

// 安排课程
int arrange_course(ComputerRoomSchedule *room, int day, int period,
                   int schedule_id, int course_id, int teacher_id) {
    if (day < 0 || day >= DAYS_PER_WEEK || period < 0 || period >= PERIODS_PER_DAY) {
        return 0;  // 无效时间
    }

    if (room->schedule_table[day][period].schedule_id != -1) {
        return 0;  // 时间冲突
    }

    room->schedule_table[day][period].schedule_id = schedule_id;
    room->schedule_table[day][period].course_id = course_id;
    room->schedule_table[day][period].teacher_id = teacher_id;
    return 1;
}

// 查询某时段是否空闲
int is_slot_free(ComputerRoomSchedule *room, int day, int period) {
    if (day < 0 || day >= DAYS_PER_WEEK || period < 0 || period >= PERIODS_PER_DAY) {
        return 0;
    }
    return room->schedule_table[day][period].schedule_id == -1;
}

// 获取某天的课程安排
void get_day_schedule(ComputerRoomSchedule *room, int day, TimeSlot **result) {
    *result = room->schedule_table[day];
}

// 计算某机房的使用率
float calculate_usage_rate(ComputerRoomSchedule *room) {
    int total_slots = DAYS_PER_WEEK * PERIODS_PER_DAY;
    int used_slots = 0;

    for (int day = 0; day < DAYS_PER_WEEK; day++) {
        for (int period = 0; period < PERIODS_PER_DAY; period++) {
            if (room->schedule_table[day][period].schedule_id != -1) {
                used_slots++;
            }
        }
    }

    return (float)used_slots / total_slots * 100;
}
```

### 3.5 性能对比

| 操作 | 优化前 | 优化后 |
|------|--------|--------|
| 查询某时段 | O(n) | O(1) |
| 查询某天 | O(n) | O(1) 获取整行 |
| 统计利用率 | O(n) | O(1) 直接计算 |
| 安排课程 | O(n) | O(1) |

---

## 4. 课程依赖关系 - 有向图

### 4.1 问题分析

某些课程可能有前置课程要求，需要在其他课程完成之后才能安排。

### 4.2 优化方案

使用有向图存储课程之间的依赖关系，支持拓扑排序检测循环依赖。

### 4.3 数据结构定义

```c
// 邻接表节点
typedef struct GraphNode {
    int course_id;
    struct GraphNode *next;
} GraphNode;

// 课程图
typedef struct {
    GraphNode **adjacency_list;  // 邻接表
    int *in_degree;              // 入度数组
    int course_count;
} CourseGraph;
```

### 4.4 核心操作

```c
// 添加依赖边
void add_dependency(CourseGraph *graph, int from_course, int to_course) {
    GraphNode *node = malloc(sizeof(GraphNode));
    node->course_id = from_course;
    node->next = graph->adjacency_list[to_course];
    graph->adjacency_list[to_course] = node;
    graph->in_degree[from_course]++;
}

// 检测循环依赖（拓扑排序）
int detect_cycle(CourseGraph *graph, int *sorted_order) {
    Queue *q = queue_create();
    int count = 0;

    // 将入度为0的节点入队
    for (int i = 0; i < graph->course_count; i++) {
        if (graph->in_degree[i] == 0) {
            queue_push(q, i);
        }
    }

    while (!queue_is_empty(q)) {
        int course = queue_pop(q);
        sorted_order[count++] = course;

        GraphNode *node = graph->adjacency_list[course];
        while (node) {
            graph->in_degree[node->course_id]--;
            if (graph->in_degree[node->course_id] == 0) {
                queue_push(q, node->course_id);
            }
            node = node->next;
        }
    }

    queue_free(q);
    return count == graph->course_count ? 1 : 0;  // 1表示有循环依赖
}
```

---

## 5. 实现优先级建议

对于 Demo 项目，建议按以下顺序实现：

| 优先级 | 数据结构 | 实现难度 | 性能提升 | 原因 |
|--------|----------|----------|----------|------|
| ⭐⭐⭐ | 哈希表 | 中等 | 冲突检测 O(n) → O(1) | 冲突检测是最高频操作，提升最明显 |
| ⭐⭐ | 二维数组 | 简单 | 多维度 O(n) → O(1) | 结构简单，代码量少，查询效率高 |
| ⭐ | AVL 树 | 较难 | 范围查询 O(n) → O(log n) | 实现较复杂，可先使用简单结构代替 |
| ⭐ | 有向图 | 中等 | 依赖检测 | 扩展功能，非核心需求 |

---

## 6. 综合优化架构

```c
// 排课系统数据结构
typedef struct {
    // 持久化存储
    sqlite3 *db;

    // 内存索引
    ScheduleIndex *schedule_index;          // 哈希表索引
    ScheduleNode *schedule_tree;            // AVL树索引
    WeeklyScheduleMap *weekly_map;          // 二维数组
    CourseGraph *course_graph;              // 课程依赖图

} SchedulingSystem;

// 初始化系统
int init_system(SchedulingSystem *sys, const char *db_path) {
    // 初始化数据库
    if (sqlite3_open(db_path, &sys->db) != SQLITE_OK) {
        return 0;
    }

    // 加载所有数据到内存索引
    load_all_schedules(sys);

    return 1;
}
```

---

## 7. 时间复杂度总结

| 数据结构 | 空间复杂度 | 冲突检测 | 范围查询 | 插入 | 删除 |
|----------|------------|----------|----------|------|------|
| 链表（无索引） | O(n) | O(n) | O(n) | O(1) | O(n) |
| 哈希表 | O(n) | O(1) | O(n) | O(1) | O(1) |
| 二维数组 | O(固定) | O(1) | O(1) 行 | O(1) | O(1) |
| AVL 树 | O(n) | O(log n) | O(log n + k) | O(log n) | O(log n) |
