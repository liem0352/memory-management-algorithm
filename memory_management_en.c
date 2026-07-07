#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define MAX_MEMORY 1024  // 内存总大小(KB)
#define MAX_BLOCKS 50    // 最大分区数

// 分区结构体
typedef struct {
    int start_addr;     // 起始地址
    int size;          // 分区大小
    int status;        // 状态: 0-空闲, 1-已分配
    int job_id;        // 作业ID，空闲时为-1
} MemoryBlock;

// 全局变量
MemoryBlock free_blocks[MAX_BLOCKS];   // 空闲区表
MemoryBlock allocated_blocks[MAX_BLOCKS]; // 已分配表
int free_count = 0;     // 空闲区数量
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

// 初始化内存
void initialize_memory() {
    // 初始化整个内存为一个空闲区
    free_blocks[0].start_addr = 0;
    free_blocks[0].size = MAX_MEMORY;
    free_blocks[0].status = 0;
    free_blocks[0].job_id = -1;
    free_count = 1;
    
    allocated_count = 0;
    printf("Memory initialized! Total memory size: %d KB\n", MAX_MEMORY);
}

// 显示内存状态
void display_memory() {
    printf("\n========== Memory Status ==========\n");
    printf("Free Blocks Table:\n");
    printf("No.\tStart\t\tSize(KB)\tStatus\n");
    for (int i = 0; i < free_count; i++) {
        printf("%d\t%d\t\t%d\t%s\n", i+1, free_blocks[i].start_addr, 
               free_blocks[i].size, free_blocks[i].status == 0 ? "Free" : "Allocated");
    }
    
    printf("\nAllocated Blocks Table:\n");
    printf("No.\tJob ID\tStart\t\tSize(KB)\n");
    for (int i = 0; i < allocated_count; i++) {
        printf("%d\t%d\t%d\t\t%d\n", i+1, allocated_blocks[i].job_id,
               allocated_blocks[i].start_addr, allocated_blocks[i].size);
    }
    printf("================================\n");
}

// 显示菜单
void display_menu() {
    printf("\n========== Variable Partition Memory Management ==========\n");
    printf("1. Select allocation algorithm\n");
    printf("2. Allocate memory\n");
    printf("3. Free memory\n");
    printf("4. Display memory status\n");
    printf("5. Initialize memory\n");
    printf("0. Exit\n");
    printf("========================================================\n");
    printf("Please select operation: ");
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
        printf("Allocation failed: Not enough continuous memory space!\n");
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
    
    printf("Memory allocation successful! Job %d allocated %d KB memory, start address: %d\n", 
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
            
            printf("Memory free successful! Job %d memory has been released\n", job_id);
            break;
        }
    }
    
    if (!found) {
        printf("Free failed: Job %d not found\n", job_id);
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
    
    printf("========== Variable Partition Memory Management System ==========\n");
    initialize_memory();
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                printf("Please select allocation algorithm:\n");
                printf("1. First Fit\n");
                printf("2. Best Fit\n");
                printf("3. Worst Fit\n");
                printf("Select: ");
                scanf("%d", &algorithm);
                if (algorithm >= 1 && algorithm <= 3) {
                    current_algorithm = algorithm;
                    const char* algo_names[] = {"", "First Fit", "Best Fit", "Worst Fit"};
                    printf("Switched to %s\n", algo_names[algorithm]);
                } else {
                    printf("Invalid selection!\n");
                }
                break;
                
            case 2:
                printf("Please enter job ID and required memory size(KB): ");
                scanf("%d %d", &job_id, &size);
                allocate_memory(job_id, size);
                break;
                
            case 3:
                printf("Please enter job ID to free: ");
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
                printf("Thank you for using!\n");
                exit(0);
                
            default:
                printf("Invalid selection!\n");
        }
    }
    
    return 0;
}