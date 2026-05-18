#ifndef SIMULATION_H
#define SIMULATION_H

#include <stddef.h> // for size_t

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 模拟配置结构 ====================
typedef struct {
    unsigned int numParticles;   // 粒子数量
    float particleRadius;        // 粒子半径
    float cellSpacing;           // 网格间距
    unsigned int gridWidth;      // 网格列数
    unsigned int gridHeight;     // 网格行数
    float timeStep;              // 时间步长 (dt)
    float overRelaxation;        // 压力求解超松弛因子

    // 边界类型：0 = 矩形边界，1 = 圆形边界
    int boundaryType;
    // 圆形边界参数（当 boundaryType == 1 时使用）
    float circleCenterX;
    float circleCenterY;
    float circleRadius;

    // 内存分配器函数指针（若为 NULL 则使用标准库函数）
    void* (*malloc_func)(size_t size);
    void  (*free_func)(void* ptr);
    void* (*realloc_func)(void* ptr, size_t size);
} SimulationConfig;

// ==================== 移动实体结构 ====================
typedef struct {
    float x, y;          // 圆心位置（物理坐标）
    float radius;        // 半径
} MovingEntity;

// ==================== 模拟实例结构（不透明指针） ====================
typedef struct Simulation Simulation;

// ==================== 公共接口 ====================

/**
 * 创建一个新的模拟实例，并根据配置初始化内部状态。
 * 返回 NULL 表示创建失败（内存不足或参数无效）。
 */
Simulation* Simulation_Create(const SimulationConfig* config);

/**
 * 销毁模拟实例，释放所有内存。
 */
void Simulation_Destroy(Simulation* sim);

/**
 * 执行一个完整的时间步模拟。
 * @param gx  X方向重力加速度（如 0.0f）
 * @param gy  Y方向重力加速度（如 9.8f）
 */
void Simulation_Step(Simulation* sim, float gx, float gy);

/**
 * 重置模拟到初始状态（粒子位置重新按六角密堆积排列，速度清零）。
 */
void Simulation_Reset(Simulation* sim);

// ==================== 移动实体管理接口 ====================

/**
 * 添加一个移动实体（圆形障碍物）。
 * 返回实体的索引，失败返回 -1。
 */
int Simulation_AddMovingEntity(Simulation* sim, float x, float y, float radius);

/**
 * 更新指定移动实体的位置。
 */
void Simulation_SetMovingEntityPosition(Simulation* sim, int index, float x, float y);

/**
 * 获取移动实体的数量。
 */
int Simulation_GetMovingEntityCount(const Simulation* sim);

/**
 * 获取移动实体的信息（通过输出参数返回）。
 * 返回 1 成功，0 失败。
 */
int Simulation_GetMovingEntity(const Simulation* sim, int index, float* x, float* y, float* radius);

/**
 * 移除指定移动实体。
 */
void Simulation_RemoveMovingEntity(Simulation* sim, int index);

// ==================== 数据访问接口 ====================

unsigned int Simulation_GetParticleCount(const Simulation* sim);
float Simulation_GetParticleX(const Simulation* sim, unsigned int idx);
float Simulation_GetParticleY(const Simulation* sim, unsigned int idx);

unsigned int Simulation_GetGridWidth(const Simulation* sim);
unsigned int Simulation_GetGridHeight(const Simulation* sim);
unsigned int Simulation_GetCellType(const Simulation* sim, unsigned int x, unsigned int y); // 返回 0:FLUID, 1:AIR, 2:SOLID

// 单元格类型常量
#define SIM_CELL_FLUID  0U
#define SIM_CELL_AIR    1U
#define SIM_CELL_SOLID  2U

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_H
