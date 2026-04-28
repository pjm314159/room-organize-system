#include "ui.h"

void ui_show_main_menu(void) {
    printf("\n");
    printf("========================================\n");
    printf("          机房排课系统\n");
    printf("========================================\n");
    printf("1. 基础数据管理\n");
    printf("2. 排课管理\n");
    printf("3. 数据统计\n");
    printf("0. 退出系统\n");
    printf("========================================\n");
}

int ui_get_main_choice(void) {
    return ui_input_int_range("请选择操作: ", 0, 3);
}

void ui_show_data_menu(void) {
    printf("\n");
    printf("========== 基础数据管理 ==========\n");
    printf("1. 机房管理\n");
    printf("2. 课程管理\n");
    printf("3. 教师管理\n");
    printf("0. 返回上级菜单\n");
    printf("================================\n");
}

int ui_get_data_choice(void) {
    return ui_input_int_range("请选择: ", 0, 3);
}

void ui_show_room_menu(void) {
    printf("\n");
    printf("========== 机房管理 ==========\n");
    printf("1. 添加机房\n");
    printf("2. 查看机房列表\n");
    printf("3. 修改机房信息\n");
    printf("4. 删除机房\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_room_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}

void ui_show_course_menu(void) {
    printf("\n");
    printf("========== 课程管理 ==========\n");
    printf("1. 添加课程\n");
    printf("2. 查看课程列表\n");
    printf("3. 修改课程信息\n");
    printf("4. 删除课程\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_course_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}

void ui_show_teacher_menu(void) {
    printf("\n");
    printf("========== 教师管理 ==========\n");
    printf("1. 添加教师\n");
    printf("2. 查看教师列表\n");
    printf("3. 修改教师信息\n");
    printf("4. 删除教师\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_teacher_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}

void ui_show_schedule_menu(void) {
    printf("\n");
    printf("========== 排课管理 ==========\n");
    printf("1. 创建排课\n");
    printf("2. 查询排课\n");
    printf("3. 修改排课\n");
    printf("4. 删除排课\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_schedule_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}

void ui_input_string(const char *prompt, char *buffer, int max_len) {
    printf("%s", prompt);
    if (fgets(buffer, max_len, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

int ui_input_int(const char *prompt) {
    char buffer[100];
    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        return atoi(buffer);
    }
    return 0;
}

int ui_input_int_range(const char *prompt, int min, int max) {
    int value;
    while (1) {
        value = ui_input_int(prompt);
        if (value >= min && value <= max) {
            break;
        }
        printf("输入无效，请输入 %d-%d 之间的数字\n", min, max);
    }
    return value;
}

char ui_input_confirm(const char *prompt) {
    char buffer[10];
    printf("%s (Y/N): ", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        char c = buffer[0];
        return (c == 'Y' || c == 'y') ? 'Y' : 'N';
    }
    return 'N';
}

void ui_show_title(const char *title) {
    printf("\n");
    printf("========== %s ==========\n", title);
}

void ui_show_divider(void) {
    printf("----------------------------------------\n");
}

void ui_show_success(const char *message) {
    printf("[成功] %s\n", message);
}

void ui_show_error(const char *message) {
    printf("[错误] %s\n", message);
}

void ui_show_warning(const char *message) {
    printf("[警告] %s\n", message);
}

void ui_pause(void) {
    printf("\n按回车键继续...");
    getchar();
}

void ui_display_room(ComputerRoom *room) {
    const char *status_str = room->status == ROOM_STATUS_NORMAL ? "正常" : "维护";
    printf("|%4d | %-10s | %-15s | %4d | %-6s|\n",
           room->id, room->room_no, room->name, room->capacity, status_str);
}

void ui_display_rooms(ComputerRoom *rooms, int count) {
    if (count == 0) {
        printf("暂无机房信息\n");
        return;
    }

    ui_show_title("机房列表");
    printf("+-----+------------+-----------------+------+--------+\n");
    printf("| ID  | 机房编号    | 机房名称        | 容量 | 状态   |\n");
    printf("+-----+------------+-----------------+------+--------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_room(&rooms[i]);
    }

    printf("+-----+------------+-----------------+------+--------+\n");
    printf("共 %d 条记录\n", count);
}

void ui_display_teacher(Teacher *teacher) {
    printf("|%4d | %-10s | %-15s | %-15s|\n",
           teacher->id, teacher->teacher_no, teacher->name, teacher->phone);
}

void ui_display_teachers(Teacher *teachers, int count) {
    if (count == 0) {
        printf("暂无教师信息\n");
        return;
    }

    ui_show_title("教师列表");
    printf("+-----+------------+-----------------+-----------------+\n");
    printf("| ID  | 工号        | 姓名            | 联系电话        |\n");
    printf("+-----+------------+-----------------+-----------------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_teacher(&teachers[i]);
    }

    printf("+-----+------------+-----------------+-----------------+\n");
    printf("共 %d 条记录\n", count);
}

void ui_display_course(Course *course) {
    printf("|%4d | %-10s | %-15s | %4d |\n",
           course->id, course->course_no, course->name, course->hours);
}

void ui_display_courses(Course *courses, int count) {
    if (count == 0) {
        printf("暂无课程信息\n");
        return;
    }

    ui_show_title("课程列表");
    printf("+-----+------------+-----------------+------+\n");
    printf("| ID  | 课程编号    | 课程名称        | 课时 |\n");
    printf("+-----+------------+-----------------+------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_course(&courses[i]);
    }

    printf("+-----+------------+-----------------+------+\n");
    printf("共 %d 条记录\n", count);
}

void ui_display_schedule(Schedule *schedule) {
    printf("|%4d | %4d       | %4d          | %4d       | %4d     |\n",
           schedule->id, schedule->room_id, schedule->course_id,
           schedule->teacher_id, schedule->day_of_week, schedule->period);
}

void ui_display_schedule_detail(ScheduleDetail *detail) {
    printf("|%4d | %-10s | %-15s | %-10s | %-4s  | 第%d节  |\n",
           detail->schedule_id,
           detail->room_name,
           detail->course_name,
           detail->teacher_name,
           ui_get_day_name(detail->day_of_week),
           detail->period);
}

void ui_display_schedule_details(ScheduleDetail *details, int count) {
    if (count == 0) {
        printf("暂无排课信息\n");
        return;
    }

    ui_show_title("排课详情");
    printf("+-----+------------+-----------------+-------------+--------+--------+\n");
    printf("| ID  | 机房        | 课程            | 教师        | 星期   | 课时   |\n");
    printf("+-----+------------+-----------------+-------------+--------+--------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_schedule_detail(&details[i]);
    }

    printf("+-----+------------+-----------------+-------------+--------+--------+\n");
    printf("共 %d 条记录\n", count);
}

const char* ui_get_day_name(int day) {
    static const char *days[] = {"", "周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    if (day >= 1 && day <= 7) {
        return days[day];
    }
    return "未知";
}
