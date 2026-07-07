#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>

#define MAX_MEMORY 1024  // 内存总大小(KB)
#define MAX_BLOCKS 50  // 最大分区数

// 分区结构体
typedef struct {
  int start_addr;  // 起始地址
  int size;  // 分区大小
  int status;  // 状态: 0-空闲, 1-已分配
  int job_id;  // 作业ID，空闲时为-1
} MemoryBlock;

// 全局变量
MemoryBlock free_blocks[MAX_BLOCKS];  // 空闲区表
MemoryBlock allocated_blocks[MAX_BLOCKS]; // 已分配表
int free_count = 0;  // 空闲区数量
int allocated_count = 0; // 已分配区数量
int current_algorithm = 1; // 当前算法: 1-首次适应, 2-最佳适应, 3-最坏适应

// 函数声明
void initialize_memory();
void display_memory();
void display_menu();
int allocate_memory(int job_id, int size);
void free_memory(int job_id);
int first_fit(int size);
int best_fit(int size);
int worst_fit(int size);
void merge_free_blocks();
void set_console_encoding();

// 设置控制台编码为UTF-8
void set_console_encoding() {
  // 设置控制台输出编码为UTF-8
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);
  // 设置本地化环境
  setlocale(LC_ALL, "zh_CN.UTF-8");
}

// 初始化内存
void initialize_memory() {
  // 初始化整个内存为一个空闲区
  free_blocks[0].start_addr = 0;
  free_blocks[0].size = MAX_MEMORY;
  free_blocks[0].status = 0;
  free_blocks[0].job_id = -1;
  free_count = 1;
  
  allocated_count = 0;
  printf("内存初始化完成！总内存大小：%d KB\n", MAX_MEMORY);
}

// 显示内存状态
void display_memory() {
  printf("\n========== 内存状态 ==========\n");
  printf("空闲区表：\n");
  printf("序号\t起始地址\t大小(KB)\t状态\n");
  for (int i = 0; i < free_count; i++) {
  printf("%d\t%d\t\t%d\t%s\n", i+1, free_blocks[i].start_addr, 
  free_blocks[i].size, free_blocks[i].status == 0 ? "空闲" : "已分配");
  }
  
  printf("\n已分配表：\n");
  printf("序号\t作业ID\t起始地址\t大小(KB)\n");
  for (int i = 0; i < allocated_count; i++) {
  printf("%d\t%d\t%d\t\t%d\n", i+1, allocated_blocks[i].job_id,
  allocated_blocks[i].start_addr, allocated_blocks[i].size);
  }
  printf("===============================\n");
}

// 显示菜单
void display_menu() {
  printf("\n========== 可变式分区存储管理系统 ==========\n");
  printf("1. 选择分配算法\n");
  printf("2. 分配内存\n");
  printf("3. 回收内存\n");
  printf("4. 显示内存状态\n");
  printf("5. 初始化内存\n");
  printf("0. 退出\n");
  printf("==========================================\n");
  printf("请选择操作：");
}

// 首次适应算法
int first_fit(int size) {
  for (int i = 0; i < free_count; i++) {
  if (free_blocks[i].status == 0 && free_blocks[i].size >= size) {
  return i;
  }
  }
  return -1;
}

// 最佳适应算法
int best_fit(int size) {
  int best_index = -1;
  int min_size = MAX_MEMORY + 1;
  
  for (int i = 0; i < free_count; i++) {
  if (free_blocks[i].status == 0 && free_blocks[i].size >= size) {
  if (free_blocks[i].size < min_size) {
  min_size = free_blocks[i].size;
  best_index = i;
  }
  }
  }
  return best_index;
}

// 最坏适应算法
int worst_fit(int size) {
  int worst_index = -1;
  int max_size = -1;
  
  for (int i = 0; i < free_count; i++) {
  if (free_blocks[i].status == 0 && free_blocks[i].size >= size) {
  if (free_blocks[i].size > max_size) {
  max_size = free_blocks[i].size;
  worst_index = i;
  }
  }
  }
  return worst_index;
}

// 分配内存
int allocate_memory(int job_id, int size) {
  int block_index = -1;
  
  // 根据选择的算法查找合适的分区
  switch (current_algorithm) {
  case 1:
  block_index = first_fit(size);
  break;
  case 2:
  block_index = best_fit(size);
  break;
  case 3:
  block_index = worst_fit(size);
  break;
  }
  
  if (block_index == -1) {
  printf("分配失败：没有足够的连续内存空间！\n");
  return 0;
  }
  
  // 检查是否需要分割分区
  if (free_blocks[block_index].size > size) {
  // 创建新的空闲分区
  MemoryBlock new_block;
  new_block.start_addr = free_blocks[block_index].start_addr + size;
  new_block.size = free_blocks[block_index].size - size;
  new_block.status = 0;
  new_block.job_id = -1;
  
  // 更新原分区大小
  free_blocks[block_index].size = size;
  
  // 插入新分区到空闲区表
  for (int i = free_count; i > block_index + 1; i--) {
  free_blocks[i] = free_blocks[i-1];
  }
  free_blocks[block_index + 1] = new_block;
  free_count++;
  }
  
  // 更新分区状态
  free_blocks[block_index].status = 1;
  free_blocks[block_index].job_id = job_id;
  
  // 添加到已分配表
  allocated_blocks[allocated_count] = free_blocks[block_index];
  allocated_count++;
  
  printf("内存分配成功！作业%d分配了%d KB内存，起始地址：%d\n", 
  job_id, size, free_blocks[block_index].start_addr);
  return 1;
}

// 回收内存
void free_memory(int job_id) {
  int found = 0;
  
  // 在已分配表中查找要回收的作业
  for (int i = 0; i < allocated_count; i++) {
  if (allocated_blocks[i].job_id == job_id) {
  found = 1;
  
  // 在空闲区表中找到对应的分区并标记为空闲
  for (int j = 0; j < free_count; j++) {
  if (free_blocks[j].start_addr == allocated_blocks[i].start_addr) {
  free_blocks[j].status = 0;
  free_blocks[j].job_id = -1;
  break;
  }
  }
  
  // 从已分配表中删除
  for (int k = i; k < allocated_count - 1; k++) {
  allocated_blocks[k] = allocated_blocks[k + 1];
  }
  allocated_count--;
  
  printf("内存回收成功！作业%d的内存已释放\n", job_id);
  break;
  }
  }
  
  if (!found) {
  printf("回收失败：未找到作业%d\n", job_id);
  return;
  }
  
  // 合并相邻的空闲分区
  merge_free_blocks();
}

// 合并相邻的空闲分区
void merge_free_blocks() {
  for (int i = 0; i < free_count - 1; i++) {
  if (free_blocks[i].status == 0 && free_blocks[i + 1].status == 0 &&
  free_blocks[i].start_addr + free_blocks[i].size == free_blocks[i + 1].start_addr) {
  
  // 合并分区
  free_blocks[i].size += free_blocks[i + 1].size;
  
  // 删除后面的分区
  for (int j = i + 1; j < free_count - 1; j++) {
  free_blocks[j] = free_blocks[j + 1];
  }
  free_count--;
  i--; // 重新检查当前位置
  }
  }
}

int main() {
  int choice, job_id, size, algorithm;
  
  // 设置控制台编码
  set_console_encoding();
  
  printf("========== 可变式分区存储管理系统 ==========\n");
  initialize_memory();
  
  while (1) {
  display_menu();
  scanf("%d", &choice);
  
  switch (choice) {
  case 1:
  printf("请选择分配算法：\n");
  printf("1. 首次适应算法\n");
  printf("2. 最佳适应算法\n");
  printf("3. 最坏适应算法\n");
  printf("选择：");
  scanf("%d", &algorithm);
  if (algorithm >= 1 && algorithm <= 3) {
  current_algorithm = algorithm;
  const char* algo_names[] = {"", "首次适应算法", "最佳适应算法", "最坏适应算法"};
  printf("已切换到%s\n", algo_names[algorithm]);
  } else {
  printf("无效选择！\n");
  }
  break;
  
  case 2:
  printf("请输入作业ID和所需内存大小(KB)：");
  scanf("%d %d", &job_id, &size);
  allocate_memory(job_id, size);
  break;
  
  case 3:
  printf("请输入要回收的作业ID：");
  scanf("%d", &job_id);
  free_memory(job_id);
  break;
  
  case 4:
  display_memory();
  break;
  
  case 5:
  initialize_memory();
  break;
  
  case 0:
  printf("感谢使用！\n");
  exit(0);
  
  default:
  printf("无效选择！\n");
  }
  }
  
  return 0;
}