# 机房排课系统

一个简单实用的机房课程安排管理系统，使用 C 语言开发，基于 SQLite 数据库存储数据。

## 功能特性

- **终端界面** - 菜单驱动操作，简洁直观
- **机房管理** - 添加、查看、修改、删除机房信息
- **教师管理** - 教师信息的增删改查
- **课程管理** - 课程信息的完整管理
- **排课功能** - 创建排课、冲突检测、多维度查询
- **数据统计** - 机房使用率统计

## 快速开始

## 使用release的编译后产物
[release](https://github.com/pjm314159/room-organize-system/releases/)

### 编译

```bash
make
```

### 运行

```bash
bin/schedule_system
```

或使用 make：

```bash
make run
```

## 项目结构

```
├── Docs/                    # 文档目录
│   ├── overview.md          # 项目概述
│   ├── tasks.md             # 开发任务分解
│   ├── data-structure-optimization.md
│   └── windows-setup.md     # Windows 环境配置
├── src/                     # 源代码
│   ├── main.c               # 主程序入口
│   ├── types.h              # 类型定义
│   ├── db.h / db.c          # 数据库模块
│   ├── ui.h / ui.c          # 用户界面模块
│   └── logic.h / logic.c    # 业务逻辑模块
├── build/                   # 编译中间文件
├── bin/                     # 可执行文件
├── data/                    # 数据库文件
└── Makefile                 # 编译配置
```

## 环境要求

- GCC 编译器 (支持 C99)
- SQLite3 开发库

## 文档

| 文档 | 说明 |
|------|------|
| [项目概述](Docs/overview.md) | 系统架构、数据库设计、界面设计 |
| [开发任务](Docs/tasks.md) | 详细的开发任务分解和实现代码 |
| [数据结构优化](Docs/data-structure-optimization.md) | 性能优化方案 |
| [Windows 配置指南](Docs/windows-setup.md) | Windows 环境下的编译配置 |

## 主要功能演示

### 主菜单

```
========================================
          机房排课系统
========================================
1. 基础数据管理
2. 排课管理
3. 数据统计
0. 退出系统
========================================
```

### 排课冲突检测

系统会自动检测：
- 同一机房、同一时间只能有一门课程
- 同一教师、同一时间只能有一门课程

## 数据库表结构

| 表名 | 说明 |
|------|------|
| computer_room | 机房信息 |
| teacher | 教师信息 |
| course | 课程信息 |
| schedule | 排课记录 |

## 许可证

MIT License
