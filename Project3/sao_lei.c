// =====================================================================
//  扫雷 · 主文件 sao_lei.c（只有 main 函数）
//
//  按题目要求：本文件用 extern 的方式声明并使用"函数源文件"
//  （sao_lei_core.c）里的函数；界面部分则在 sao_lei_ui.c 里。
// =====================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>     // strcmp

// ---------------- 函数源文件 sao_lei_core.c 里的函数：extern 声明 ----------------
extern void InitRandom(void);                    // 设置随机种子
extern void NewGame(void);                       // 重置棋盘 + 布雷
extern void SetupShotScenario(const char *mode); // --shot 调试局面

// ---------------- 界面源文件 sao_lei_ui.c 里的入口函数：extern 声明 ----------------
extern int  UiRun(int shot, const char *shotFile);

int main(int argc, char *argv[]) {
	int shot = 0;
	const char *shotMode = "play";
	const char *shotFile = "sao_lei_shot.png";

	// --shot 调试用：生成 lose / win 局面截图，正常玩不需要管
	if (argc > 1 && strcmp(argv[1], "--shot") == 0) {
		shot = 1;
		if (argc > 2) shotMode = argv[2];
		if (argc > 3) shotFile = argv[3];
	}

	InitRandom();                            // 核心逻辑：随机种子
	NewGame();                               // 核心逻辑：重置棋盘 + 布雷
	if (shot) SetupShotScenario(shotMode);   // 核心逻辑：构造调试局面

	return UiRun(shot, shotFile);            // 界面：开窗口 + 主循环
}