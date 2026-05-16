/**
 * @file logic.c
 * @brief 业务逻辑模块实现
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2026
 * 
 * @description
 * 本文件实现了所有业务逻辑函数，负责处理用户交互、数据验证、
 * 业务规则检查（如冲突检测）等核心业务功能。
 */

#include "logic.h"
#include "ui.h"

/*============================================================================
 * 机房管理业务逻辑
 *============================================================================*/

int logic_add_room(void) {
    ComputerRoom room;

    /* 获取用户输入 */
    ui_input_string("请输入机房编号: ", room.room_no, sizeof(room.room_no));
    if (strlen(room.room_no) == 0) {
        ui_show_error("机房编号不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入机房名称: ", room.name, sizeof(room.name));
    if (strlen(room.name) == 0) {
        ui_show_error("机房名称不能为空");
        return ERROR_INVALID_INPUT;
    }

    room.capacity = ui_input_int_range("请输入容纳人数: ", 1, 500);
    room.status = ui_input_int_range("请输入状态 (0-正常, 1-维护): ", 0, 1);

    /* 调用数据库接口 */
    int result = db_create_room(&room);
    if (result == SUCCESS) {
        ui_show_success("机房添加成功");
        return SUCCESS;
    } else if (result == ERROR_DUPLICATE) {
        ui_show_error("机房编号已存在");
        return ERROR_DUPLICATE;
    } else {
        ui_show_error("添加失败");
        return FAILURE;
    }
}

int logic_list_rooms(void) {
    ComputerRoom *rooms = NULL;
    int count = 0;

    int result = db_get_all_rooms(&rooms, &count);
    if (result == SUCCESS) {
        ui_display_rooms(rooms, count);
        free(rooms);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_update_room(void) {
    int id = ui_input_int("请输入要修改的机房ID (0返回): ");
    if (id == 0) return SUCCESS;

    /* 查询现有记录 */
    ComputerRoom room;
    if (db_get_room_by_id(id, &room) != SUCCESS) {
        ui_show_error("机房不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_room(&room);
    printf("\n请输入新信息 (直接回车保持原值):\n");

    /* 获取新值（可选） */
    char buffer[100];
    ui_input_string("机房编号: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(room.room_no, buffer);

    ui_input_string("机房名称: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(room.name, buffer);

    int capacity = ui_input_int("容纳人数: ");
    if (capacity > 0) room.capacity = capacity;

    room.status = ui_input_int_range("状态 (0-正常, 1-维护): ", 0, 1);

    /* 更新数据库 */
    int result = db_update_room(&room);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_room(void) {
    int id = ui_input_int("请输入要删除的机房ID (0返回): ");
    if (id == 0) return SUCCESS;

    ComputerRoom room;
    if (db_get_room_by_id(id, &room) != SUCCESS) {
        ui_show_error("机房不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_room(&room);

    /* 确认删除 */
    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_room(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}

/*============================================================================
 * 教师管理业务逻辑
 *============================================================================*/

int logic_add_teacher(void) {
    Teacher teacher;

    ui_input_string("请输入工号: ", teacher.teacher_no, sizeof(teacher.teacher_no));
    if (strlen(teacher.teacher_no) == 0) {
        ui_show_error("工号不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入姓名: ", teacher.name, sizeof(teacher.name));
    if (strlen(teacher.name) == 0) {
        ui_show_error("姓名不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入联系电话: ", teacher.phone, sizeof(teacher.phone));

    int result = db_create_teacher(&teacher);
    if (result == SUCCESS) {
        ui_show_success("教师添加成功");
        return SUCCESS;
    } else if (result == ERROR_DUPLICATE) {
        ui_show_error("工号已存在");
        return ERROR_DUPLICATE;
    } else {
        ui_show_error("添加失败");
        return FAILURE;
    }
}

int logic_list_teachers(void) {
    Teacher *teachers = NULL;
    int count = 0;

    int result = db_get_all_teachers(&teachers, &count);
    if (result == SUCCESS) {
        ui_display_teachers(teachers, count);
        free(teachers);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_update_teacher(void) {
    int id = ui_input_int("请输入要修改的教师ID (0返回): ");
    if (id == 0) return SUCCESS;

    Teacher teacher;
    if (db_get_teacher_by_id(id, &teacher) != SUCCESS) {
        ui_show_error("教师不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_teacher(&teacher);
    printf("\n请输入新信息 (直接回车保持原值):\n");

    char buffer[100];
    ui_input_string("工号: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(teacher.teacher_no, buffer);

    ui_input_string("姓名: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(teacher.name, buffer);

    ui_input_string("联系电话: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(teacher.phone, buffer);

    int result = db_update_teacher(&teacher);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_teacher(void) {
    int id = ui_input_int("请输入要删除的教师ID (0返回): ");
    if (id == 0) return SUCCESS;

    Teacher teacher;
    if (db_get_teacher_by_id(id, &teacher) != SUCCESS) {
        ui_show_error("教师不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_teacher(&teacher);

    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_teacher(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}

/*============================================================================
 * 课程管理业务逻辑
 *============================================================================*/

int logic_add_course(void) {
    Course course;

    ui_input_string("请输入课程编号: ", course.course_no, sizeof(course.course_no));
    if (strlen(course.course_no) == 0) {
        ui_show_error("课程编号不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入课程名称: ", course.name, sizeof(course.name));
    if (strlen(course.name) == 0) {
        ui_show_error("课程名称不能为空");
        return ERROR_INVALID_INPUT;
    }

    /* 选择授课教师 */
    Teacher *teachers = NULL;
    int teacher_count = 0;
    db_get_all_teachers(&teachers, &teacher_count);

    if (teacher_count == 0) {
        ui_show_error("请先添加教师");
        free(teachers);
        return ERROR_NOT_FOUND;
    }

    ui_display_teachers(teachers, teacher_count);
    course.teacher_id = ui_input_int_range("请选择教师ID: ", 1, 1000);

    Teacher tmp;
    if (db_get_teacher_by_id(course.teacher_id, &tmp) != SUCCESS) {
        ui_show_error("教师不存在");
        free(teachers);
        return ERROR_NOT_FOUND;
    }

    course.hours = ui_input_int_range("请输入所需课时数: ", 1, 100);

    int result = db_create_course(&course);
    free(teachers);

    if (result == SUCCESS) {
        ui_show_success("课程添加成功");
        return SUCCESS;
    } else if (result == ERROR_DUPLICATE) {
        ui_show_error("课程编号已存在");
        return ERROR_DUPLICATE;
    } else {
        ui_show_error("添加失败");
        return FAILURE;
    }
}

int logic_list_courses(void) {
    Course *courses = NULL;
    int count = 0;

    int result = db_get_all_courses(&courses, &count);
    if (result == SUCCESS) {
        ui_display_courses(courses, count);
        free(courses);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_update_course(void) {
    int id = ui_input_int("请输入要修改的课程ID (0返回): ");
    if (id == 0) return SUCCESS;

    Course course;
    if (db_get_course_by_id(id, &course) != SUCCESS) {
        ui_show_error("课程不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_course(&course);
    printf("\n请输入新信息 (直接回车保持原值):\n");

    char buffer[100];
    ui_input_string("课程编号: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(course.course_no, buffer);

    ui_input_string("课程名称: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(course.name, buffer);

    course.hours = ui_input_int("所需课时数: ");
    if (course.hours <= 0) course.hours = 1;

    int result = db_update_course(&course);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_course(void) {
    int id = ui_input_int("请输入要删除的课程ID (0返回): ");
    if (id == 0) return SUCCESS;

    Course course;
    if (db_get_course_by_id(id, &course) != SUCCESS) {
        ui_show_error("课程不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_course(&course);

    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_course(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}

/*============================================================================
 * 排课管理业务逻辑
 *============================================================================*/

int logic_add_schedule(void) {
    Schedule schedule;

    /* 选择机房 */
    ComputerRoom *rooms = NULL;
    int room_count = 0;
    db_get_all_rooms(&rooms, &room_count);

    if (room_count == 0) {
        ui_show_error("请先添加机房");
        return ERROR_NOT_FOUND;
    }

    ui_display_rooms(rooms, room_count);
    schedule.room_id = ui_input_int_range("请选择机房ID: ", 1, 1000);

    ComputerRoom room;
    if (db_get_room_by_id(schedule.room_id, &room) != SUCCESS) {
        ui_show_error("机房不存在");
        free(rooms);
        return ERROR_NOT_FOUND;
    }
    free(rooms);

    /* 选择课程 */
    Course *courses = NULL;
    int course_count = 0;
    db_get_all_courses(&courses, &course_count);

    if (course_count == 0) {
        ui_show_error("请先添加课程");
        return ERROR_NOT_FOUND;
    }

    ui_display_courses(courses, course_count);
    schedule.course_id = ui_input_int_range("请选择课程ID: ", 1, 1000);

    Course course;
    if (db_get_course_by_id(schedule.course_id, &course) != SUCCESS) {
        ui_show_error("课程不存在");
        free(courses);
        return ERROR_NOT_FOUND;
    }
    schedule.teacher_id = course.teacher_id;  /* 教师从课程中获取 */
    free(courses);

    /* 选择时间 */
    printf("上课时间:\n");
    printf("星期 (1-7, 1=周一, 7=周日): ");
    schedule.day_of_week = ui_input_int_range("", 1, 7);
    schedule.period = ui_input_int_range("第几节课 (1-8): ", 1, 8);

    /* 冲突检测 */
    if (db_check_room_conflict(schedule.room_id, schedule.day_of_week, schedule.period) == ERROR_CONFLICT) {
        ui_show_error("该机房此时段已有排课");
        return ERROR_CONFLICT;
    }

    if (db_check_teacher_conflict(schedule.teacher_id, schedule.day_of_week, schedule.period) == ERROR_CONFLICT) {
        ui_show_error("该教师此时段已有课程");
        return ERROR_CONFLICT;
    }

    /* 创建排课记录 */
    int result = db_create_schedule(&schedule);
    if (result == SUCCESS) {
        ui_show_success("排课创建成功");
        return SUCCESS;
    }

    ui_show_error("创建失败");
    return FAILURE;
}

int logic_list_schedules(void) {
    ScheduleDetail *details = NULL;
    int count = 0;

    int result = db_get_schedule_details(&details, &count);
    if (result == SUCCESS) {
        ui_display_schedule_details(details, count);
        free(details);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_query_schedules(void) {
    printf("\n========== 排课查询 ==========\n");
    printf("1. 按机房查询\n");
    printf("2. 按教师查询\n");
    printf("3. 按星期查询\n");
    printf("0. 返回\n");

    int choice = ui_input_int_range("请选择: ", 0, 3);

    Schedule *schedules = NULL;
    int count = 0;
    int result = SUCCESS;

    switch (choice) {
        case 1: {
            /* 按机房查询 */
            ComputerRoom *rooms = NULL;
            int room_count = 0;
            db_get_all_rooms(&rooms, &room_count);

            if (room_count == 0) {
                ui_show_error("暂无机房");
                break;
            }

            ui_display_rooms(rooms, room_count);
            int room_id = ui_input_int_range("请选择机房ID: ", 1, 1000);
            free(rooms);

            result = db_get_schedules_by_room(room_id, &schedules, &count);
            break;
        }
        case 2: {
            /* 按教师查询 */
            Teacher *teachers = NULL;
            int teacher_count = 0;
            db_get_all_teachers(&teachers, &teacher_count);

            if (teacher_count == 0) {
                ui_show_error("暂无教师");
                break;
            }

            ui_display_teachers(teachers, teacher_count);
            int teacher_id = ui_input_int_range("请选择教师ID: ", 1, 1000);
            free(teachers);

            result = db_get_schedules_by_teacher(teacher_id, &schedules, &count);
            break;
        }
        case 3: {
            /* 按星期查询 */
            int day = ui_input_int_range("请输入星期 (1-7): ", 1, 7);
            result = db_get_schedules_by_day(day, &schedules, &count);
            break;
        }
        default:
            return SUCCESS;
    }

    /* 显示查询结果 */
    if (result == SUCCESS) {
        if (count == 0) {
            printf("没有找到排课记录\n");
        } else {
            printf("\n找到 %d 条记录:\n", count);
            for (int i = 0; i < count; i++) {
                printf("  排课ID: %d, 机房ID: %d, 课程ID: %d, 教师ID: %d, 星期: %d, 课时: %d\n",
                       schedules[i].id, schedules[i].room_id, schedules[i].course_id,
                       schedules[i].teacher_id, schedules[i].day_of_week, schedules[i].period);
            }
        }
        free(schedules);
    } else {
        ui_show_error("查询失败");
    }

    return result;
}

int logic_update_schedule(void) {
    int id = ui_input_int("请输入要修改的排课ID (0返回): ");
    if (id == 0) return SUCCESS;

    Schedule schedule;
    if (db_get_schedule_by_id(id, &schedule) != SUCCESS) {
        ui_show_error("排课不存在");
        return ERROR_NOT_FOUND;
    }

    printf("\n当前信息:\n");
    printf("  机房ID: %d, 课程ID: %d, 教师ID: %d\n", schedule.room_id, schedule.course_id, schedule.teacher_id);
    printf("  时间: 星期%d, 第%d节课\n", schedule.day_of_week, schedule.period);

    /* 只允许修改时间 */
    int new_day = ui_input_int_range("新星期 (1-7): ", 1, 7);
    int new_period = ui_input_int_range("新课次 (1-8): ", 1, 8);

    /* 冲突检测 */
    if (db_check_room_conflict(schedule.room_id, new_day, new_period) == ERROR_CONFLICT) {
        ui_show_error("该机房此时段已有排课");
        return ERROR_CONFLICT;
    }

    if (db_check_teacher_conflict(schedule.teacher_id, new_day, new_period) == ERROR_CONFLICT) {
        ui_show_error("该教师此时段已有课程");
        return ERROR_CONFLICT;
    }

    schedule.day_of_week = new_day;
    schedule.period = new_period;

    int result = db_update_schedule(&schedule);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_schedule(void) {
    int id = ui_input_int("请输入要删除的排课ID (0返回): ");
    if (id == 0) return SUCCESS;

    Schedule schedule;
    if (db_get_schedule_by_id(id, &schedule) != SUCCESS) {
        ui_show_error("排课不存在");
        return ERROR_NOT_FOUND;
    }

    printf("当前信息:\n");
    printf("  机房ID: %d, 课程ID: %d, 教师ID: %d, 星期: %d, 课时: %d\n",
           schedule.room_id, schedule.course_id, schedule.teacher_id,
           schedule.day_of_week, schedule.period);

    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_schedule(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}

/*============================================================================
 * 统计功能
 *============================================================================*/

int logic_show_statistics(void) {
    ui_show_title("数据统计");

    /* 机房统计 */
    ComputerRoom *rooms = NULL;
    int room_count = 0;
    db_get_all_rooms(&rooms, &room_count);

    printf("\n机房数量: %d\n", room_count);

    if (room_count > 0) {
        printf("\n机房使用情况:\n");
        printf("+------------+-----------------+------+\n");
        printf("| 机房名称    | 机房编号        | 使用次数 |\n");
        printf("+------------+-----------------+------+\n");

        for (int i = 0; i < room_count; i++) {
            int usage = db_get_room_usage_count(rooms[i].id);
            printf("| %-10s | %-15s | %d     |\n",
                   rooms[i].name, rooms[i].room_no, usage);
        }
        printf("+------------+-----------------+------+\n");

        free(rooms);
    }

    /* 教师统计 */
    Teacher *teachers = NULL;
    int teacher_count = 0;
    db_get_all_teachers(&teachers, &teacher_count);
    printf("\n教师数量: %d\n", teacher_count);
    free(teachers);

    /* 课程统计 */
    Course *courses = NULL;
    int course_count = 0;
    db_get_all_courses(&courses, &course_count);
    printf("课程数量: %d\n", course_count);
    free(courses);

    /* 排课统计 */
    Schedule *schedules = NULL;
    int schedule_count = 0;
    db_get_all_schedules(&schedules, &schedule_count);
    printf("排课数量: %d\n", schedule_count);
    free(schedules);

    /* 使用率计算 */
    if (schedule_count > 0 && room_count > 0) {
        float usage_rate = (float)schedule_count / (room_count * DAYS_PER_WEEK * PERIODS_PER_DAY) * 100;
        printf("\n总体使用率: %.1f%%\n", usage_rate);
    }

    return SUCCESS;
}
