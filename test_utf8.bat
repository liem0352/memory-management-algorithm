@echo off
chcp 65001 >nul
echo 测试可变式分区存储管理系统
echo ==================================
echo.

echo 启动程序...
echo 程序将演示内存分配和回收功能
echo.
echo 示例操作步骤：
echo 1. 首先选择分配算法（选项1）
echo 2. 为一些作业分配内存（选项2）
echo  - 尝试：作业1，大小200KB
echo  - 尝试：作业2，大小300KB  
echo  - 尝试：作业3，大小150KB
echo 3. 显示内存状态（选项4）
echo 4. 回收一些内存（选项3）
echo  - 尝试：作业2
echo 5. 再次显示内存状态查看合并效果
echo.
echo 启动程序...
echo.

memory_management_utf8.exe

pause