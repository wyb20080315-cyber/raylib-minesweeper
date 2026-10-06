// =====================================================================
//  扫雷 · 公共头文件 sao_lei.h
//  这里放"所有函数声明"以及各源文件共享的全局数据声明。
//
//  本工程的源文件分工：
//    sao_lei_core.c   核心逻辑（棋盘数据 + 布雷 / 展开 / 判定），不含 raylib
//    sao_lei_ui.c     界面（全部 raylib 内容：窗口、字体、绘制、鼠标、主循环）
//    sao_lei.c        主文件（只有 main：解析命令行参数 + extern 调用上面两边）
// =====================================================================
#ifndef SAO_LEI_H
#define SAO_LEI_H

#define BOARD_N    9      // 9 x 9 棋盘
#define MINE_TOTAL 10     // 10 颗雷

#define STATE_PLAY 0
#define STATE_WIN  1
#define STATE_LOSE 2

// ---------------- 核心数据（定义在 sao_lei_core.c） ----------------
extern char   qi_pan[BOARD_N][BOARD_N];    // 显示棋盘：'*'未翻开，' '空，'1'~'8'数字，'0'雷
extern char   qi_pan2[BOARD_N][BOARD_N];   // 雷区布置：'0'雷，'#'已展开的空区
extern int    g_state;                     // STATE_PLAY / STATE_WIN / STATE_LOSE
extern int    g_explodedX, g_explodedY;     // 踩到的那颗雷（界面用它标红）
extern double g_startTime, g_endTime;      // 计时

// ---------------- 界面数据（定义在 sao_lei_ui.c） ----------------
extern int qi_flag[BOARD_N][BOARD_N];      // 插旗标记
extern int g_flagsUsed;                    // 已插旗数量

// ---------------- 函数声明：核心逻辑（sao_lei_core.c） ----------------
void InitRandom(void);                                                  // 设置随机种子
void NewGame(void);                                                     // 重置棋盘 + 布雷
void SetupShotScenario(const char *mode);                                // --shot 调试局面
void mai_lei(char qi_pan2[][BOARD_N]);                                   // 原控制台版：布雷
void pan_duan2(int x, int y, char qi_pan[][BOARD_N], char qi_pan2[][BOARD_N]);  // 原控制台版：展开
int  pan_duan(int x, int y, char qi_pan[][BOARD_N], char qi_pan2[][BOARD_N]);   // 原控制台版：判定
void RevealCell(int cx, int cy, double now);                             // 翻开一格（now 由界面传入）

// ---------------- 函数声明：界面（sao_lei_ui.c） ----------------
int  UiRun(int shot, const char *shotFile);                              // 开窗口 + 主循环

#endif // SAO_LEI_H