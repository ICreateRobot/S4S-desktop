#include "simulation.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>  // 用于默认的 malloc/free/realloc

// ==================== 内部数据结构定义 ====================

struct Simulation
{
    // 配置参数（只读）
    unsigned int numParticles;
    float particleRadius;
    float cellSpacing;
    unsigned int gridWidth;
    unsigned int gridHeight;
    float timeStep;
    float overRelaxation;
    int boundaryType;                                 // 边界类型
    float circleCenterX, circleCenterY, circleRadius; // 圆形边界参数

    // 派生常量
    float invertSpacing;      // 1.0 / cellSpacing
    float particleInvSpacing; // 1.0 / (2.2 * particleRadius)
    unsigned int totalCells;  // gridWidth * gridHeight

    // 精细网格尺寸（用于粒子推开）
    unsigned int fineGridWidth;  // gridWidth * 2
    unsigned int fineGridHeight; // gridHeight * 2
    unsigned int fineTotalCells; // fineGridWidth * fineGridHeight

    // 状态数组
    float *particlePos; // [numParticles * 2]
    float *particleVel; // [numParticles * 2]

    float *uVel;    // [totalCells]
    float *vVel;    // [totalCells]
    float *uPrev;   // [totalCells]
    float *vPrev;   // [totalCells]
    float *uWeight; // [totalCells]
    float *vWeight; // [totalCells]

    float *pressure;        // [totalCells]
    float *particleDensity; // [totalCells]
    float *solidMask;       // [totalCells]  静态固体掩膜（0=固体，1=可流动）
    unsigned int *cellType; // [totalCells]  当前帧的单元格类型

    // 粒子推开用
    unsigned int *numCellParticles;  // [fineTotalCells]
    unsigned int *firstCellParticle; // [fineTotalCells + 1]
    unsigned int *cellParticleIds;   // [numParticles]

    float particleRestDensity; // 静止密度

    // 移动实体
    MovingEntity *movingEntities; // 动态数组
    int movingEntityCount;
    int movingEntityCapacity;

    // 内存分配器函数指针
    void* (*malloc_func)(size_t);
    void  (*free_func)(void*);
    void* (*realloc_func)(void*, size_t);
};

// ==================== 辅助函数 ====================

static int clamp_index(int value, int minVal, int maxVal)
{
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

static float clampf_local(float value, float minVal, float maxVal)
{
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// 设置静态固体掩膜（根据边界类型）
static void setup_solid_mask(Simulation *sim)
{
    unsigned int w = sim->gridWidth;
    unsigned int h = sim->gridHeight;
    float spacing = sim->cellSpacing;

    if (sim->boundaryType == 0)
    { // 矩形边界
        for (unsigned int x = 0; x < w; x++)
        {
            for (unsigned int y = 0; y < h; y++)
            {
                float s = 1.0f;
                if (x == 0U || x == w - 1U || y == 0U || y == h - 1U)
                    s = 0.0f;
                sim->solidMask[x * h + y] = s;
            }
        }
    }
    else
    { // 圆形边界
        float cx = sim->circleCenterX;
        float cy = sim->circleCenterY;
        float r = sim->circleRadius;
        for (unsigned int x = 0; x < w; x++)
        {
            for (unsigned int y = 0; y < h; y++)
            {
                float gx = (x + 0.5f) * spacing;
                float gy = (y + 0.5f) * spacing;
                float dx = gx - cx;
                float dy = gy - cy;
                float dist = sqrtf(dx * dx + dy * dy);
                sim->solidMask[x * h + y] = (dist <= r) ? 1.0f : 0.0f;
            }
        }
    }
}

// 按六角密堆积排列粒子（考虑边界类型，确保粒子在可流动区域）
static void init_particle_positions(Simulation *sim)
{
    const float h = sim->cellSpacing;
    const float r = sim->particleRadius;
    const float dx = 2.0f * r;
    const float dy = 0.86602540378f * dx; // sqrt(3)/2

    unsigned int w = sim->gridWidth;
    unsigned int hh = sim->gridHeight;
    unsigned int p_num = 0;
    for (unsigned int i = 0; i < w && p_num < sim->numParticles; i++)
    {
        for (unsigned int j = 0; j < hh && p_num < sim->numParticles; j++)
        {
            float px = h + r + dx * (float)i + ((j % 2U == 0U) ? 0.0f : r);
            float py = h + r + dy * (float)j;

            int xi = (int)floorf(px * sim->invertSpacing);
            int yi = (int)floorf(py * sim->invertSpacing);
            if (xi < 0 || xi >= (int)w || yi < 0 || yi >= (int)hh) continue;
            if (sim->solidMask[xi * hh + yi] == 0.0f) continue;

            sim->particlePos[2 * p_num] = px;
            sim->particlePos[2 * p_num + 1] = py;
            p_num++;
        }
    }

    if (p_num < sim->numParticles)
    {
        for (; p_num < sim->numParticles; p_num++)
        {
            sim->particlePos[2 * p_num] = h + r;
            sim->particlePos[2 * p_num + 1] = h + r;
        }
    }
}

// ==================== 内部步骤函数 ====================

static void integrate_particles(Simulation *sim, float gx, float gy)
{
    const float dt = sim->timeStep;

    float minX = sim->cellSpacing + sim->particleRadius;
    float maxX = (sim->gridWidth - 1U) * sim->cellSpacing - sim->particleRadius;
    float minY = sim->cellSpacing + sim->particleRadius;
    float maxY = (sim->gridHeight - 1U) * sim->cellSpacing - sim->particleRadius;

    for (unsigned int i = 0; i < sim->numParticles; i++)
    {
        sim->particleVel[2 * i] += gx * dt;
        sim->particleVel[2 * i + 1] += gy * dt;
        sim->particlePos[2 * i] += sim->particleVel[2 * i] * dt;
        sim->particlePos[2 * i + 1] += sim->particleVel[2 * i + 1] * dt;

        float x = sim->particlePos[2 * i];
        float y = sim->particlePos[2 * i + 1];

        if (sim->boundaryType == 0)
        {
            if (x < minX) { x = minX; sim->particleVel[2 * i] = 0.0f; }
            if (x > maxX) { x = maxX; sim->particleVel[2 * i] = 0.0f; }
            if (y < minY) { y = minY; sim->particleVel[2 * i + 1] = 0.0f; }
            if (y > maxY) { y = maxY; sim->particleVel[2 * i + 1] = 0.0f; }
        }
        else
        {
            float cx = sim->circleCenterX;
            float cy = sim->circleCenterY;
            float r = sim->circleRadius - sim->particleRadius;
            float dx = x - cx;
            float dy = y - cy;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > r)
            {
                if (dist > 0.0f)
                {
                    float nx = dx / dist;
                    float ny = dy / dist;
                    x = cx + nx * r;
                    y = cy + ny * r;
                    float vn = sim->particleVel[2 * i] * nx + sim->particleVel[2 * i + 1] * ny;
                    if (vn > 0.0f)
                    {
                        sim->particleVel[2 * i] -= 2.0f * vn * nx;
                        sim->particleVel[2 * i + 1] -= 2.0f * vn * ny;
                    }
                }
                else
                {
                    x = cx + r;
                    y = cy;
                }
            }
        }

        for (int e = 0; e < sim->movingEntityCount; e++)
        {
            MovingEntity *ent = &sim->movingEntities[e];
            float dx = x - ent->x;
            float dy = y - ent->y;
            float dist = sqrtf(dx * dx + dy * dy);
            float minDist = ent->radius + sim->particleRadius;
            if (dist < minDist)
            {
                if (dist > 0.0f)
                {
                    float nx = dx / dist;
                    float ny = dy / dist;
                    x = ent->x + nx * minDist;
                    y = ent->y + ny * minDist;
                    float vn = sim->particleVel[2 * i] * nx + sim->particleVel[2 * i + 1] * ny;
                    if (vn > 0.0f)
                    {
                        sim->particleVel[2 * i] -= 2.0f * vn * nx;
                        sim->particleVel[2 * i + 1] -= 2.0f * vn * ny;
                    }
                }
                else
                {
                    x = ent->x + minDist;
                    y = ent->y;
                }
            }
        }

        sim->particlePos[2 * i] = x;
        sim->particlePos[2 * i + 1] = y;
    }
}

static void push_particles_apart(Simulation *sim, unsigned int nIters)
{
    const float invSpacing = sim->particleInvSpacing;
    const unsigned int fw = sim->fineGridWidth;
    const unsigned int fh = sim->fineGridHeight;

    for (unsigned int iter = 0; iter < nIters; iter++)
    {
        memset(sim->numCellParticles, 0, sim->fineTotalCells * sizeof(unsigned int));

        for (unsigned int i = 0; i < sim->numParticles; i++)
        {
            int xi = clamp_index((int)floorf(sim->particlePos[2 * i] * invSpacing), 0, (int)fw - 1);
            int yi = clamp_index((int)floorf(sim->particlePos[2 * i + 1] * invSpacing), 0, (int)fh - 1);
            sim->numCellParticles[xi * fh + yi]++;
        }

        unsigned int first = 0;
        for (unsigned int i = 0; i < sim->fineTotalCells; i++)
        {
            first += sim->numCellParticles[i];
            sim->firstCellParticle[i] = first;
        }
        sim->firstCellParticle[sim->fineTotalCells] = first;

        for (unsigned int i = 0; i < sim->numParticles; i++)
        {
            int xi = clamp_index((int)floorf(sim->particlePos[2 * i] * invSpacing), 0, (int)fw - 1);
            int yi = clamp_index((int)floorf(sim->particlePos[2 * i + 1] * invSpacing), 0, (int)fh - 1);
            unsigned int cellNr = xi * fh + yi;
            sim->firstCellParticle[cellNr]--;
            sim->cellParticleIds[sim->firstCellParticle[cellNr]] = i;
        }

        const float minDist = 2.0f * sim->particleRadius;
        const float minDist2 = minDist * minDist;

        for (unsigned int i = 0; i < sim->numParticles; i++)
        {
            float px = sim->particlePos[2 * i];
            float py = sim->particlePos[2 * i + 1];

            int pxi = (int)floorf(px * invSpacing);
            int pyi = (int)floorf(py * invSpacing);
            int x0 = (pxi - 1 > 0) ? pxi - 1 : 0;
            int y0 = (pyi - 1 > 0) ? pyi - 1 : 0;
            int x1 = (pxi + 1 < (int)fw - 1) ? pxi + 1 : (int)fw - 1;
            int y1 = (pyi + 1 < (int)fh - 1) ? pyi + 1 : (int)fh - 1;

            for (int xi = x0; xi <= x1; xi++)
            {
                for (int yi = y0; yi <= y1; yi++)
                {
                    unsigned int cellNr = xi * fh + yi;
                    unsigned int firstIdx = sim->firstCellParticle[cellNr];
                    unsigned int lastIdx = sim->firstCellParticle[cellNr + 1U];
                    for (unsigned int j = firstIdx; j < lastIdx; j++)
                    {
                        unsigned int id = sim->cellParticleIds[j];
                        if (id == i) continue;

                        float qx = sim->particlePos[2 * id];
                        float qy = sim->particlePos[2 * id + 1];
                        float dx = qx - px;
                        float dy = qy - py;
                        float d2 = dx * dx + dy * dy;
                        if (d2 > minDist2 || d2 == 0.0f) continue;

                        float d = sqrtf(d2);
                        float s = 0.5f * (minDist - d) / d;
                        dx *= s; dy *= s;
                        sim->particlePos[2 * i] -= dx;
                        sim->particlePos[2 * i + 1] -= dy;
                        sim->particlePos[2 * id] += dx;
                        sim->particlePos[2 * id + 1] += dy;
                    }
                }
            }
        }
    }
}

static void density_update(Simulation *sim)
{
    memset(sim->particleDensity, 0, sim->totalCells * sizeof(float));

    const float h = sim->cellSpacing;
    const float h1 = sim->invertSpacing;
    const float h2 = 0.5f * h;

    for (unsigned int i = 0; i < sim->numParticles; i++)
    {
        float x = clampf_local(sim->particlePos[2 * i], h, (sim->gridWidth - 1U) * h);
        float y = clampf_local(sim->particlePos[2 * i + 1], h, (sim->gridHeight - 1U) * h);

        int x0 = (int)floorf((x - h2) * h1);
        int y0 = (int)floorf((y - h2) * h1);
        x0 = clamp_index(x0, 0, (int)sim->gridWidth - 2);
        y0 = clamp_index(y0, 0, (int)sim->gridHeight - 2);
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        float tx = ((x - h2) - x0 * h) * h1;
        float ty = ((y - h2) - y0 * h) * h1;
        float sx = 1.0f - tx;
        float sy = 1.0f - ty;

        unsigned int idx00 = x0 * sim->gridHeight + y0;
        unsigned int idx10 = x1 * sim->gridHeight + y0;
        unsigned int idx11 = x1 * sim->gridHeight + y1;
        unsigned int idx01 = x0 * sim->gridHeight + y1;

        sim->particleDensity[idx00] += sx * sy;
        sim->particleDensity[idx10] += tx * sy;
        sim->particleDensity[idx11] += tx * ty;
        sim->particleDensity[idx01] += sx * ty;
    }

    if (sim->particleRestDensity == 0.0f)
    {
        float sum = 0.0f;
        unsigned int numFluid = 0;
        for (unsigned int i = 0; i < sim->totalCells; i++)
        {
            if (sim->cellType[i] == SIM_CELL_FLUID)
            {
                sum += sim->particleDensity[i];
                numFluid++;
            }
        }
        if (numFluid > 0U)
        {
            sim->particleRestDensity = sum / (float)numFluid;
        }
    }
}

static void particles_to_grid(Simulation *sim)
{
    memcpy(sim->uPrev, sim->uVel, sim->totalCells * sizeof(float));
    memcpy(sim->vPrev, sim->vVel, sim->totalCells * sizeof(float));

    memset(sim->uVel, 0, sim->totalCells * sizeof(float));
    memset(sim->vVel, 0, sim->totalCells * sizeof(float));
    memset(sim->uWeight, 0, sim->totalCells * sizeof(float));
    memset(sim->vWeight, 0, sim->totalCells * sizeof(float));

    for (unsigned int i = 0; i < sim->totalCells; i++)
    {
        sim->cellType[i] = (sim->solidMask[i] == 0.0f) ? SIM_CELL_SOLID : SIM_CELL_AIR;
    }

    for (unsigned int i = 0; i < sim->numParticles; i++)
    {
        int xi = clamp_index((int)floorf(sim->particlePos[2 * i] * sim->invertSpacing), 0, (int)sim->gridWidth - 1);
        int yi = clamp_index((int)floorf(sim->particlePos[2 * i + 1] * sim->invertSpacing), 0, (int)sim->gridHeight - 1);
        unsigned int idx = xi * sim->gridHeight + yi;
        if (sim->cellType[idx] == SIM_CELL_AIR)
            sim->cellType[idx] = SIM_CELL_FLUID;
    }

    float spacing = sim->cellSpacing;
    for (int e = 0; e < sim->movingEntityCount; e++)
    {
        MovingEntity *ent = &sim->movingEntities[e];
        int minX = (int)floorf((ent->x - ent->radius) * sim->invertSpacing);
        int maxX = (int)floorf((ent->x + ent->radius) * sim->invertSpacing);
        int minY = (int)floorf((ent->y - ent->radius) * sim->invertSpacing);
        int maxY = (int)floorf((ent->y + ent->radius) * sim->invertSpacing);
        minX = clamp_index(minX, 0, (int)sim->gridWidth - 1);
        maxX = clamp_index(maxX, 0, (int)sim->gridWidth - 1);
        minY = clamp_index(minY, 0, (int)sim->gridHeight - 1);
        maxY = clamp_index(maxY, 0, (int)sim->gridHeight - 1);

        for (int x = minX; x <= maxX; x++)
        {
            for (int y = minY; y <= maxY; y++)
            {
                float gx = (x + 0.5f) * spacing;
                float gy = (y + 0.5f) * spacing;
                float dx = gx - ent->x;
                float dy = gy - ent->y;
                if (dx * dx + dy * dy <= ent->radius * ent->radius)
                {
                    sim->cellType[x * sim->gridHeight + y] = SIM_CELL_SOLID;
                }
            }
        }
    }

    const float h = sim->cellSpacing;
    const float h1 = sim->invertSpacing;
    const float h2 = 0.5f * h;

    for (int component = 0; component < 2; component++)
    {
        float dx = (component == 0) ? 0.0f : h2;
        float dy = (component == 0) ? h2 : 0.0f;
        float *f = (component == 0) ? sim->uVel : sim->vVel;
        float *w = (component == 0) ? sim->uWeight : sim->vWeight;

        for (unsigned int i = 0; i < sim->numParticles; i++)
        {
            float x = clampf_local(sim->particlePos[2 * i], h, (sim->gridWidth - 1U) * h);
            float y = clampf_local(sim->particlePos[2 * i + 1], h, (sim->gridHeight - 1U) * h);

            int x0 = (int)floorf((x - dx) * h1);
            int y0 = (int)floorf((y - dy) * h1);
            x0 = clamp_index(x0, 0, (int)sim->gridWidth - 2);
            y0 = clamp_index(y0, 0, (int)sim->gridHeight - 2);
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            float tx = ((x - dx) - x0 * h) * h1;
            float ty = ((y - dy) - y0 * h) * h1;
            float sx = 1.0f - tx;
            float sy = 1.0f - ty;

            float w0 = sx * sy;
            float w1 = tx * sy;
            float w2 = tx * ty;
            float w3 = sx * ty;

            float pv = sim->particleVel[2 * i + (unsigned int)component];
            unsigned int nr0 = x0 * sim->gridHeight + y0;
            unsigned int nr1 = x1 * sim->gridHeight + y0;
            unsigned int nr2 = x1 * sim->gridHeight + y1;
            unsigned int nr3 = x0 * sim->gridHeight + y1;

            f[nr0] += pv * w0; w[nr0] += w0;
            f[nr1] += pv * w1; w[nr1] += w1;
            f[nr2] += pv * w2; w[nr2] += w2;
            f[nr3] += pv * w3; w[nr3] += w3;
        }

        for (unsigned int i = 0; i < sim->totalCells; i++)
        {
            if (w[i] > 0.0f) f[i] /= w[i];
        }

        for (unsigned int x = 0; x < sim->gridWidth; x++)
        {
            for (unsigned int y = 0; y < sim->gridHeight; y++)
            {
                unsigned int idx = x * sim->gridHeight + y;
                unsigned int solid = (sim->cellType[idx] == SIM_CELL_SOLID) ? 1U : 0U;
                if (component == 0)
                {
                    unsigned int leftSolid = (x > 0U && sim->cellType[(x - 1) * sim->gridHeight + y] == SIM_CELL_SOLID) ? 1U : 0U;
                    if (solid || leftSolid) sim->uVel[idx] = sim->uPrev[idx];
                }
                else
                {
                    unsigned int bottomSolid = (y > 0U && sim->cellType[x * sim->gridHeight + (y - 1)] == SIM_CELL_SOLID) ? 1U : 0U;
                    if (solid || bottomSolid) sim->vVel[idx] = sim->vPrev[idx];
                }
            }
        }
    }
}

static void compute_grid_forces(Simulation *sim, unsigned int nIters)
{
    memset(sim->pressure, 0, sim->totalCells * sizeof(float));
    memcpy(sim->uPrev, sim->uVel, sim->totalCells * sizeof(float));
    memcpy(sim->vPrev, sim->vVel, sim->totalCells * sizeof(float));

    const float cp = 1000.0f * sim->cellSpacing / sim->timeStep;

    for (unsigned int iter = 0; iter < nIters; iter++)
    {
        for (unsigned int x = 1; x < sim->gridWidth - 1U; x++)
        {
            for (unsigned int y = 1; y < sim->gridHeight - 1U; y++)
            {
                unsigned int center = x * sim->gridHeight + y;
                if (sim->cellType[center] != SIM_CELL_FLUID) continue;

                unsigned int left = (x - 1) * sim->gridHeight + y;
                unsigned int right = (x + 1) * sim->gridHeight + y;
                unsigned int bottom = x * sim->gridHeight + (y - 1);
                unsigned int top = x * sim->gridHeight + (y + 1);

                float sx0 = sim->solidMask[left];
                float sx1 = sim->solidMask[right];
                float sy0 = sim->solidMask[bottom];
                float sy1 = sim->solidMask[top];
                float s = sx0 + sx1 + sy0 + sy1;
                if (s == 0.0f) continue;

                float div = sim->uVel[right] - sim->uVel[center] + sim->vVel[top] - sim->vVel[center];

                if (sim->particleRestDensity > 0.0f)
                {
                    float compression = sim->particleDensity[center] - sim->particleRestDensity;
                    if (compression > 0.0f) div -= compression;
                }

                float p = -div / s;
                p *= sim->overRelaxation;
                sim->pressure[center] += cp * p;

                sim->uVel[center] -= sx0 * p;
                sim->uVel[right] += sx1 * p;
                sim->vVel[center] -= sy0 * p;
                sim->vVel[top] += sy1 * p;
            }
        }
    }
}

static void grid_to_particles(Simulation *sim)
{
    const float flipRatio = 0.9f;
    const float h = sim->cellSpacing;
    const float h1 = sim->invertSpacing;
    const float h2 = 0.5f * h;

    for (int component = 0; component < 2; component++)
    {
        float dx = (component == 0) ? 0.0f : h2;
        float dy = (component == 0) ? h2 : 0.0f;
        float *f = (component == 0) ? sim->uVel : sim->vVel;
        float *prevF = (component == 0) ? sim->uPrev : sim->vPrev;
        int offset = (component == 0) ? (int)sim->gridHeight : 1;

        for (unsigned int i = 0; i < sim->numParticles; i++)
        {
            float x = clampf_local(sim->particlePos[2 * i], h, (sim->gridWidth - 1U) * h);
            float y = clampf_local(sim->particlePos[2 * i + 1], h, (sim->gridHeight - 1U) * h);

            int x0 = clamp_index((int)floorf((x - dx) * h1), 0, (int)sim->gridWidth - 2);
            int y0 = clamp_index((int)floorf((y - dy) * h1), 0, (int)sim->gridHeight - 2);
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            float tx = ((x - dx) - x0 * h) * h1;
            float ty = ((y - dy) - y0 * h) * h1;
            float sx = 1.0f - tx;
            float sy = 1.0f - ty;

            float d0 = sx * sy;
            float d1 = tx * sy;
            float d2 = tx * ty;
            float d3 = sx * ty;

            unsigned int nr0 = x0 * sim->gridHeight + y0;
            unsigned int nr1 = x1 * sim->gridHeight + y0;
            unsigned int nr2 = x1 * sim->gridHeight + y1;
            unsigned int nr3 = x0 * sim->gridHeight + y1;

            int valid0 = (sim->cellType[nr0] != SIM_CELL_AIR ||
                          ((int)nr0 - offset >= 0 && sim->cellType[(unsigned int)((int)nr0 - offset)] != SIM_CELL_AIR)) ? 1 : 0;
            int valid1 = (sim->cellType[nr1] != SIM_CELL_AIR ||
                          ((int)nr1 - offset >= 0 && sim->cellType[(unsigned int)((int)nr1 - offset)] != SIM_CELL_AIR)) ? 1 : 0;
            int valid2 = (sim->cellType[nr2] != SIM_CELL_AIR ||
                          ((int)nr2 - offset >= 0 && sim->cellType[(unsigned int)((int)nr2 - offset)] != SIM_CELL_AIR)) ? 1 : 0;
            int valid3 = (sim->cellType[nr3] != SIM_CELL_AIR ||
                          ((int)nr3 - offset >= 0 && sim->cellType[(unsigned int)((int)nr3 - offset)] != SIM_CELL_AIR)) ? 1 : 0;

            float d = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;
            if (d <= 0.0f) continue;

            float picV = (valid0 * d0 * f[nr0] + valid1 * d1 * f[nr1] + valid2 * d2 * f[nr2] + valid3 * d3 * f[nr3]) / d;
            float corr = (valid0 * d0 * (f[nr0] - prevF[nr0]) + valid1 * d1 * (f[nr1] - prevF[nr1]) +
                          valid2 * d2 * (f[nr2] - prevF[nr2]) + valid3 * d3 * (f[nr3] - prevF[nr3])) / d;
            float oldV = sim->particleVel[2 * i + (unsigned int)component];
            float flipV = oldV + corr;
            sim->particleVel[2 * i + (unsigned int)component] = (1.0f - flipRatio) * picV + flipRatio * flipV;
        }
    }
}

// ==================== 公共接口实现 ====================

Simulation *Simulation_Create(const SimulationConfig *config)
{
    if (!config) return NULL;
    if (config->numParticles == 0 || config->gridWidth == 0 || config->gridHeight == 0)
        return NULL;

    // 确定分配器函数（若 config 中为 NULL，则使用标准库函数）
    void* (*my_malloc)(size_t)   = config->malloc_func ? config->malloc_func : malloc;
    void  (*my_free)(void*)      = config->free_func   ? config->free_func   : free;
    void* (*my_realloc)(void*, size_t) = config->realloc_func ? config->realloc_func : realloc;

    // 分配主结构体
    Simulation *sim = (Simulation *)my_malloc(sizeof(Simulation));
    if (!sim) return NULL;

    // 将分配器函数存入 sim
    sim->malloc_func = my_malloc;
    sim->free_func = my_free;
    sim->realloc_func = my_realloc;

    // 复制基本参数
    sim->numParticles = config->numParticles;
    sim->particleRadius = config->particleRadius;
    sim->cellSpacing = config->cellSpacing;
    sim->gridWidth = config->gridWidth;
    sim->gridHeight = config->gridHeight;
    sim->timeStep = config->timeStep;
    sim->overRelaxation = config->overRelaxation;
    sim->boundaryType = config->boundaryType;
    sim->circleCenterX = config->circleCenterX;
    sim->circleCenterY = config->circleCenterY;
    sim->circleRadius = config->circleRadius;

    // 派生常量
    sim->invertSpacing = 1.0f / sim->cellSpacing;
    sim->particleInvSpacing = 1.0f / (2.2f * sim->particleRadius);
    sim->totalCells = sim->gridWidth * sim->gridHeight;
    sim->fineGridWidth = sim->gridWidth * 2U;
    sim->fineGridHeight = sim->gridHeight * 2U;
    sim->fineTotalCells = sim->fineGridWidth * sim->fineGridHeight;

    // 动态分配数组（使用 sim 中的分配器）
    #define ALLOC_OR_FAIL(ptr, size)                       \
        do {                                                \
            ptr = (float *)sim->malloc_func((size) * sizeof(float)); \
            if (!ptr) goto cleanup;                         \
        } while (0)

    #define ALLOC_UINT(ptr, size)                           \
        do {                                                 \
            ptr = (unsigned int *)sim->malloc_func((size) * sizeof(unsigned int)); \
            if (!ptr) goto cleanup;                          \
        } while (0)

    ALLOC_OR_FAIL(sim->particlePos, sim->numParticles * 2);
    ALLOC_OR_FAIL(sim->particleVel, sim->numParticles * 2);
    ALLOC_OR_FAIL(sim->uVel, sim->totalCells);
    ALLOC_OR_FAIL(sim->vVel, sim->totalCells);
    ALLOC_OR_FAIL(sim->uPrev, sim->totalCells);
    ALLOC_OR_FAIL(sim->vPrev, sim->totalCells);
    ALLOC_OR_FAIL(sim->uWeight, sim->totalCells);
    ALLOC_OR_FAIL(sim->vWeight, sim->totalCells);
    ALLOC_OR_FAIL(sim->pressure, sim->totalCells);
    ALLOC_OR_FAIL(sim->particleDensity, sim->totalCells);
    ALLOC_OR_FAIL(sim->solidMask, sim->totalCells);
    ALLOC_UINT(sim->cellType, sim->totalCells);
    ALLOC_UINT(sim->numCellParticles, sim->fineTotalCells);
    ALLOC_UINT(sim->firstCellParticle, sim->fineTotalCells + 1);
    ALLOC_UINT(sim->cellParticleIds, sim->numParticles);

    #undef ALLOC_OR_FAIL
    #undef ALLOC_UINT

    // 初始清零
    memset(sim->particleVel, 0, sim->numParticles * 2 * sizeof(float));
    memset(sim->uVel, 0, sim->totalCells * sizeof(float));
    memset(sim->vVel, 0, sim->totalCells * sizeof(float));
    memset(sim->uPrev, 0, sim->totalCells * sizeof(float));
    memset(sim->vPrev, 0, sim->totalCells * sizeof(float));
    memset(sim->pressure, 0, sim->totalCells * sizeof(float));
    memset(sim->particleDensity, 0, sim->totalCells * sizeof(float));
    sim->particleRestDensity = 0.0f;

    // 移动实体数组初始化
    sim->movingEntities = NULL;
    sim->movingEntityCount = 0;
    sim->movingEntityCapacity = 0;

    // 设置固体掩膜
    setup_solid_mask(sim);

    // 初始化粒子位置
    init_particle_positions(sim);

    return sim;

cleanup:
    // 使用 sim 的 free_func 释放已分配的内存（注意 sim->free_func 已设置）
    if (sim->particlePos) sim->free_func(sim->particlePos);
    if (sim->particleVel) sim->free_func(sim->particleVel);
    if (sim->uVel) sim->free_func(sim->uVel);
    if (sim->vVel) sim->free_func(sim->vVel);
    if (sim->uPrev) sim->free_func(sim->uPrev);
    if (sim->vPrev) sim->free_func(sim->vPrev);
    if (sim->uWeight) sim->free_func(sim->uWeight);
    if (sim->vWeight) sim->free_func(sim->vWeight);
    if (sim->pressure) sim->free_func(sim->pressure);
    if (sim->particleDensity) sim->free_func(sim->particleDensity);
    if (sim->solidMask) sim->free_func(sim->solidMask);
    if (sim->cellType) sim->free_func(sim->cellType);
    if (sim->numCellParticles) sim->free_func(sim->numCellParticles);
    if (sim->firstCellParticle) sim->free_func(sim->firstCellParticle);
    if (sim->cellParticleIds) sim->free_func(sim->cellParticleIds);
    sim->free_func(sim);
    return NULL;
}

void Simulation_Destroy(Simulation *sim)
{
    if (!sim) return;
    sim->free_func(sim->particlePos);
    sim->free_func(sim->particleVel);
    sim->free_func(sim->uVel);
    sim->free_func(sim->vVel);
    sim->free_func(sim->uPrev);
    sim->free_func(sim->vPrev);
    sim->free_func(sim->uWeight);
    sim->free_func(sim->vWeight);
    sim->free_func(sim->pressure);
    sim->free_func(sim->particleDensity);
    sim->free_func(sim->solidMask);
    sim->free_func(sim->cellType);
    sim->free_func(sim->numCellParticles);
    sim->free_func(sim->firstCellParticle);
    sim->free_func(sim->cellParticleIds);
    sim->free_func(sim->movingEntities);
    sim->free_func(sim);
}

void Simulation_Step(Simulation *sim, float gx, float gy)
{
    if (!sim) return;
    integrate_particles(sim, gx, gy);
    push_particles_apart(sim, 2);
    particles_to_grid(sim);
    density_update(sim);
    compute_grid_forces(sim, 20);
    grid_to_particles(sim);
}

void Simulation_Reset(Simulation *sim)
{
    if (!sim) return;
    memset(sim->particleVel, 0, sim->numParticles * 2 * sizeof(float));
    init_particle_positions(sim);
    memset(sim->uVel, 0, sim->totalCells * sizeof(float));
    memset(sim->vVel, 0, sim->totalCells * sizeof(float));
    memset(sim->uPrev, 0, sim->totalCells * sizeof(float));
    memset(sim->vPrev, 0, sim->totalCells * sizeof(float));
    memset(sim->pressure, 0, sim->totalCells * sizeof(float));
    memset(sim->particleDensity, 0, sim->totalCells * sizeof(float));
    sim->particleRestDensity = 0.0f;
}

// 移动实体管理
int Simulation_AddMovingEntity(Simulation *sim, float x, float y, float radius)
{
    if (!sim) return -1;
    if (sim->movingEntityCount >= sim->movingEntityCapacity)
    {
        int newCap = (sim->movingEntityCapacity == 0) ? 4 : sim->movingEntityCapacity * 2;
        MovingEntity *newArray = (MovingEntity *)sim->realloc_func(sim->movingEntities,
                                                                    newCap * sizeof(MovingEntity));
        if (!newArray) return -1;
        sim->movingEntities = newArray;
        sim->movingEntityCapacity = newCap;
    }
    int idx = sim->movingEntityCount++;
    sim->movingEntities[idx].x = x;
    sim->movingEntities[idx].y = y;
    sim->movingEntities[idx].radius = radius;
    return idx;
}

void Simulation_SetMovingEntityPosition(Simulation *sim, int index, float x, float y)
{
    if (!sim || index < 0 || index >= sim->movingEntityCount) return;
    sim->movingEntities[index].x = x;
    sim->movingEntities[index].y = y;
}

int Simulation_GetMovingEntityCount(const Simulation *sim)
{
    return sim ? sim->movingEntityCount : 0;
}

int Simulation_GetMovingEntity(const Simulation *sim, int index, float *x, float *y, float *radius)
{
    if (!sim || index < 0 || index >= sim->movingEntityCount) return 0;
    if (x) *x = sim->movingEntities[index].x;
    if (y) *y = sim->movingEntities[index].y;
    if (radius) *radius = sim->movingEntities[index].radius;
    return 1;
}

void Simulation_RemoveMovingEntity(Simulation *sim, int index)
{
    if (!sim || index < 0 || index >= sim->movingEntityCount) return;
    if (index < sim->movingEntityCount - 1)
    {
        sim->movingEntities[index] = sim->movingEntities[sim->movingEntityCount - 1];
    }
    sim->movingEntityCount--;
}

// 数据访问接口
unsigned int Simulation_GetParticleCount(const Simulation *sim)
{
    return sim ? sim->numParticles : 0;
}

float Simulation_GetParticleX(const Simulation *sim, unsigned int idx)
{
    if (!sim || idx >= sim->numParticles) return 0.0f;
    return sim->particlePos[2 * idx];
}

float Simulation_GetParticleY(const Simulation *sim, unsigned int idx)
{
    if (!sim || idx >= sim->numParticles) return 0.0f;
    return sim->particlePos[2 * idx + 1];
}

unsigned int Simulation_GetGridWidth(const Simulation *sim)
{
    return sim ? sim->gridWidth : 0;
}

unsigned int Simulation_GetGridHeight(const Simulation *sim)
{
    return sim ? sim->gridHeight : 0;
}

unsigned int Simulation_GetCellType(const Simulation *sim, unsigned int x, unsigned int y)
{
    if (!sim || x >= sim->gridWidth || y >= sim->gridHeight) return SIM_CELL_AIR;
    return sim->cellType[x * sim->gridHeight + y];
}
