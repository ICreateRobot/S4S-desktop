/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Description  : 流体模拟 OLED 渲染（128×128 SH1107）
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "app_fluid_oled.h"
#include "hardware/esp_oled/hw_esp_oled.h"
#include "hardware/gyro/hw_gyro.h"

/* simulation.h 是纯 C 头文件，C++ 中需用 extern "C" 包裹 */
extern "C" {
#include "simulation.h"
}

/* ============================================================
 *  模拟参数（针对 128×128 OLED 调整）
 *
 *  坐标系：cellSpacing=8，gridWidth=16 → 世界宽度 = 16×8 = 128px
 *          粒子坐标直接对应 OLED 像素坐标，无需额外缩放
 * ============================================================ */
#define FLUID_GRID_W         8U     /* 网格列数（16×8=128px） */
#define FLUID_GRID_H         8U     /* 网格行数（16×8=128px） */
#define FLUID_CELL_SPACING   16.0f   /* 每格像素大小 */
#define FLUID_NUM_PARTICLES  50U   /* 粒子数（ESP32 约占 20KB，可酌情增减） */
#define FLUID_PARTICLE_R     4.0f   /* 粒子半径（像素） */
#define FLUID_TIME_STEP      0.08f /* 时间步长（≈60Hz） */
#define FLUID_OVER_RELAX     1.9f   /* 压力超松弛因子 */
#define FLUID_GRAVITY_SCALE  200.0f /* 加速度(g) → 像素/s² 比例 */

/* ============================================================
 *  内部状态
 * ============================================================ */
static Simulation *s_sim = NULL;

/* ============================================================
 *  公共接口实现
 * ============================================================ */

void app_fluid_oled_begin(void)
{
    SimulationConfig cfg;

    /* 清零结构体，避免未初始化字段引起问题 */
    memset(&cfg, 0, sizeof(cfg));

    cfg.numParticles   = FLUID_NUM_PARTICLES;
    cfg.particleRadius = FLUID_PARTICLE_R;
    cfg.cellSpacing    = FLUID_CELL_SPACING;
    cfg.gridWidth      = FLUID_GRID_W;
    cfg.gridHeight     = FLUID_GRID_H;
    cfg.timeStep       = FLUID_TIME_STEP;
    cfg.overRelaxation = FLUID_OVER_RELAX;
    cfg.boundaryType   = 0;   /* 矩形边界，外圈单元格为固体墙 */

    /* 使用默认 malloc/free（ESP32 堆足够） */
    cfg.malloc_func  = NULL;
    cfg.free_func    = NULL;
    cfg.realloc_func = NULL;

    s_sim = Simulation_Create(&cfg);
}

void app_fluid_oled_update(void)
{
    if (!s_sim) return;

    /* ---------- 1. 读取陀螺仪加速度作为重力方向 ----------
     *  LSM6DS3 坐标系（芯片面朝上，丝印朝前）：
     *    AccelX：向右为正
     *    AccelY：向上为正（含重力分量）
     *    AccelZ：垂直屏幕向外为正
     *
     *  OLED 坐标系：x 向右，y 向下
     *  因此：gx = +AccelX，gy = -AccelY
     *  根据实际安装方向调整符号即可
     * ---------------------------------------------------- */
    float ax = -hw_gyro.readFloatAccelX();
    float ay =  hw_gyro.readFloatAccelY();  /* Y 轴取反适配屏幕向下 */

    float gx = ax * FLUID_GRAVITY_SCALE;
    float gy = ay * FLUID_GRAVITY_SCALE;

    /* ---------- 2. 推进一帧流体模拟 ---------- */
    Simulation_Step(s_sim, gx, gy);

    /* ---------- 3. 渲染到 OLED ---------- */
    hw_esp_oled.clear_screen();

    // 根据 FLUID_PARTICLE_R 计算像素大小
    // particleRadius 是物理半径（像素单位），直接取整用于绘制
    const int r = (int)FLUID_PARTICLE_R;

    unsigned int n = Simulation_GetParticleCount(s_sim);
    for (unsigned int i = 0; i < n; i++)
    {
        int px = (int)Simulation_GetParticleX(s_sim, i) - r;  // 左上角起点
        int py = (int)Simulation_GetParticleY(s_sim, i) - r;

        if (px < 0 || px + r > 127 || py < 0 || py + r > 127) continue;

        hw_esp_oled.draw_circle(px, py, r);
    }

    /* 3b. 绘制移动实体（障碍物圆形轮廓） */
    // int entityCount = Simulation_GetMovingEntityCount(s_sim);
    // for (int i = 0; i < entityCount; i++)
    // {
    //     float ex, ey, er;
    //     if (Simulation_GetMovingEntity(s_sim, i, &ex, &ey, &er))
    //     {
    //         hw_esp_oled.draw_circle((int)ex, (int)ey, (int)er);
    //     }
    // }

    /* 3c. 绘制边框（对应固体墙单元格） */
    hw_esp_oled.draw_rect(0, 0, 127, 127);

    /* ---------- 4. 推送缓冲区到屏幕 ---------- */
    hw_esp_oled.refresh();
}

void app_fluid_oled_reset(void)
{
    if (s_sim) Simulation_Reset(s_sim);
}

void app_fluid_oled_destroy(void)
{
    if (s_sim)
    {
        Simulation_Destroy(s_sim);
        s_sim = NULL;
    }
}