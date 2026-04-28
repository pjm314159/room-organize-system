# Windows 环境配置指南

本文档介绍如何在 Windows 环境下配置、编译和运行机房排课系统。

---

## 1. 环境要求

- **操作系统**：Windows 10/11
- **编译器**：MinGW-w64 GCC 或 MSYS2
- **数据库**：SQLite3

---

## 2. 安装 MinGW-w64

### 方法一：使用 WinLibs（推荐）

1. 访问 [WinLibs](https://winlibs.com/) 下载页面
2. 下载最新的 GCC 版本（推荐 UCRT runtime 版本）
   - 例如：`winlibs-x86_64-posix-seh-gcc-13.2.0-llvm-17.0.6-mingw-w64ucrt-11.0.0-r5.7z`
3. 解压到 `C:\mingw64`（或其他目录）
4. 添加到系统环境变量 PATH：
   - 右键"此电脑" → "属性" → "高级系统设置"
   - 点击"环境变量"
   - 在"系统变量"中找到 `Path`，点击"编辑"
   - 添加：`C:\mingw64\bin`
5. 打开命令提示符，验证安装：
   ```cmd
   gcc --version
   ```

### 方法二：使用 MSYS2

1. 下载并安装 [MSYS2](https://www.msys2.org/)
2. 打开 MSYS2 终端，运行以下命令：
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-sqlite3
   ```
3. 将 `C:\msys64\mingw64\bin` 添加到系统 PATH

---

## 3. 安装 SQLite3

### 方法一：使用预编译库

1. 访问 [SQLite 下载页面](https://www.sqlite.org/download.html)
2. 下载以下文件：
   - **预编译二进制文件**：`sqlite-dll-win-x64-*.zip`
   - **源代码**：`sqlite-amalgamation-*.zip`
3. 解压后，将文件放置到 MinGW 目录：
   ```
   C:\mingw64\bin\sqlite3.dll
   C:\mingw64\lib\sqlite3.lib（或 libsqlite3.a）
   C:\mingw64\include\sqlite3.h
   C:\mingw64\include\sqlite3ext.h
   ```

### 方法二：使用 MSYS2（推荐）

如果使用 MSYS2，SQLite3 已在上一步安装：
```bash
pacman -S mingw-w64-x86_64-sqlite3
```

### 方法三：手动编译 SQLite3

1. 下载 SQLite 源码合并包：`sqlite-amalgamation-*.zip`
2. 解压后，在目录中创建编译脚本：
   ```cmd
   gcc -c sqlite3.c -o sqlite3.o
   ar rcs libsqlite3.a sqlite3.o
   ```
3. 将生成的文件复制到 MinGW 目录：
   - `libsqlite3.a` → `C:\mingw64\lib\`
   - `sqlite3.h` → `C:\mingw64\include\`

---

## 4. 编译项目

### 4.1 使用 Makefile（推荐）

打开命令提示符或 PowerShell，进入项目根目录：

```cmd
cd "d:\Project\organize system"
```

使用 make 编译：

```cmd
make
```

或使用 mingw32-make：

```cmd
mingw32-make
```

编译完成后，可执行文件位于 `bin/schedule_system.exe`。

### 4.2 使用命令行编译

```cmd
cd "d:\Project\organize system"
gcc -o bin/schedule_system.exe src/main.c src/db.c src/ui.c src/logic.c -lsqlite3 -I"C:\mingw64\include" -L"C:\mingw64\lib"
```

如果遇到链接错误，尝试静态链接：

```cmd
gcc -o bin/schedule_system.exe src/main.c src/db.c src/ui.c src/logic.c -lsqlite3 -static
```

### 4.3 常见编译问题

**问题 1：找不到 sqlite3.h**

```
fatal error: sqlite3.h: No such file or directory
```

**解决方案**：确保 SQLite3 头文件在编译器的 include 路径中，或使用 `-I` 参数指定路径。

**问题 2：链接错误 -lsqlite3**

```
cannot find -lsqlite3
```

**解决方案**：
- 确保 `libsqlite3.a` 或 `sqlite3.lib` 在 lib 目录中
- 使用 `-L` 参数指定库文件路径

**问题 3：运行时缺少 sqlite3.dll**

```
无法启动此程序，因为计算机中丢失 sqlite3.dll
```

**解决方案**：
- 将 `sqlite3.dll` 复制到可执行文件同目录
- 或使用静态链接编译

---

## 5. 运行程序

### 5.1 运行

程序会自动创建 `data` 目录，无需手动创建。

```cmd
cd "d:\Project\organize system"
bin\schedule_system.exe
```

或使用 make 运行：

```cmd
make run
```

---

## 6. 使用 Visual Studio Code 开发

### 6.1 安装扩展

推荐安装以下 VS Code 扩展：
- **C/C++** (Microsoft)
- **C/C++ Extension Pack**
- **Makefile Tools**

### 6.2 配置 tasks.json

在 `.vscode/tasks.json` 中添加编译任务：

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "gcc",
            "args": [
                "-o", "bin/schedule_system.exe",
                "src/main.c", "src/db.c", "src/ui.c", "src/logic.c",
                "-lsqlite3",
                "-Wall", "-Wextra"
            ],
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "problemMatcher": ["$gcc"],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "run",
            "type": "shell",
            "command": "./bin/schedule_system.exe",
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "dependsOn": "build"
        }
    ]
}
```

### 6.3 配置 c_cpp_properties.json

在 `.vscode/c_cpp_properties.json` 中配置 IntelliSense：

```json
{
    "configurations": [
        {
            "name": "Windows",
            "includePath": [
                "${workspaceFolder}/src",
                "C:/mingw64/include"
            ],
            "defines": ["_WIN32"],
            "compilerPath": "C:/mingw64/bin/gcc.exe",
            "cStandard": "c99",
            "intelliSenseMode": "windows-gcc-x64"
        }
    ],
    "version": 4
}
```

---

## 7. 项目目录结构

```
d:\Project\organize system\
├── Docs/                           # 文档目录
│   ├── overview.md                 # 项目概述
│   ├── tasks.md                    # 开发任务
│   ├── data-structure-optimization.md
│   └── windows-setup.md            # 本文档
├── src/                            # 源代码目录
│   ├── main.c                      # 主程序
│   ├── types.h                     # 类型定义
│   ├── db.h / db.c                 # 数据库模块
│   ├── ui.h / ui.c                 # 用户界面模块
│   └── logic.h / logic.c           # 业务逻辑模块
├── build/                          # 编译中间文件（自动生成）
├── bin/                            # 可执行文件目录（自动生成）
│   └── schedule_system.exe         # 编译生成的可执行文件
├── data/                           # 数据库文件目录（自动生成）
│   └── schedule.db                 # SQLite 数据库
└── Makefile                        # 编译配置
```

---

## 8. 快速开始

完整流程：

```cmd
# 1. 进入项目目录
cd "d:\Project\organize system"

# 2. 编译
make

# 3. 运行
bin\schedule_system.exe
```

> **注意**：程序首次运行时会自动创建 `data` 目录和数据库文件。

---

## 9. 测试数据示例

首次运行后，可以添加测试数据：

1. **添加教师**
   - 工号：T001
   - 姓名：张老师
   - 电话：13800138000

2. **添加机房**
   - 机房编号：R101
   - 机房名称：1号机房
   - 容量：50
   - 状态：0（正常）

3. **添加课程**
   - 课程编号：CS101
   - 课程名称：C语言程序设计
   - 选择教师：1
   - 课时：4

4. **创建排课**
   - 选择机房：1
   - 选择课程：1
   - 星期：1（周一）
   - 课时：1

---

## 10. 常见问题

### Q: 编译时提示 `undefined reference to sqlite3_xxx`

A: 这是链接错误，确保：
- SQLite3 库文件存在
- `-lsqlite3` 参数在源文件之后
- 库文件路径正确

### Q: 程序运行时闪退

A: 可能是数据库路径问题：
- 确保从正确目录运行程序
- 检查 `data` 目录是否存在

### Q: 中文显示乱码

A: 程序已自动设置 UTF-8 编码。如果仍有问题，请确保：
- 使用 Windows Terminal 或支持 UTF-8 的终端
- 源代码文件使用 UTF-8 编码保存

如果使用传统命令提示符，可手动设置：
```cmd
chcp 65001
```

---

## 11. 参考链接

- [MinGW-w64 官网](https://www.mingw-w64.org/)
- [WinLibs 下载](https://winlibs.com/)
- [MSYS2 官网](https://www.msys2.org/)
- [SQLite 官网](https://www.sqlite.org/)
- [SQLite 下载页面](https://www.sqlite.org/download.html)
