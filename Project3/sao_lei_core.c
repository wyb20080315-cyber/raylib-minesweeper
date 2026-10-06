// =====================================================================
//  扫雷 · 核心逻辑 sao_lei_core.c
//  与界面完全无关（不包含 raylib）。
//  其中 mai_lei / pan_duan2 / pan_duan 三个函数来自原控制台版，
//  除了把"打印结果"改成写界面状态（g_state / g_explodedX / g_explodedY），
//  算法本身一字未改。
// =====================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>     // rand / srand
#include <time.h>       // time
#include <string.h>     // strcmp
#include "sao_lei.h"

// ---------------- 核心数据定义 ----------------
char   qi_pan[BOARD_N][BOARD_N];
char   qi_pan2[BOARD_N][BOARD_N];
int    g_state = STATE_PLAY;
int    g_explodedX = -1;
int    g_explodedY = -1;
double g_startTime = 0.0;
double g_endTime = 0.0;

void InitRandom(void) {
	srand((unsigned int)time(NULL));
}

// ===================== 以下三个函数来自原控制台版 =====================
void mai_lei(char qi_pan2[][BOARD_N]) {
	int x_ = 0, y_ = 0;
	int ci_shu = 1;
	do {
		x_ = rand() % 9;
		y_ = rand() % 9;
		if ('0' == qi_pan2[x_][y_]) {
			continue;
		}
		qi_pan2[x_][y_] = '0';
		ci_shu++;
	} while (ci_shu != 11);
}

void pan_duan2(int x, int y, char qi_pan[][BOARD_N], char qi_pan2[][BOARD_N]) {
	if (x < 0 || x > 8 || y < 0 || y > 8)
		return;
	if (qi_pan2[y][x] == '#')
		return;
	int lei = 0;

	//判断传入坐标周围雷的数量
	for (int _y = y - 1;_y <= y + 1;_y++) {
		for (int _x = x - 1;_x <= x + 1;_x++) {
			if (_x > 8 || _x < 0) {
				continue;
			}
			if (_y > 8 || _y < 0) {
				continue;
			}
			if (qi_pan2[_y][_x] == '#')
				continue;
			if (_x == x && _y == y)
				continue;
			if ('0' == qi_pan2[_y][_x]) {
				lei++;
			}
		}
	}
	//如果有雷，自身变数字
	if (lei > 0) {
		qi_pan[y][x] = lei + '0';
	}
	//如果0雷，自身变空格，对周围邻居调用pan_duan2函数
	if (lei == 0) {
		qi_pan[y][x] = ' ';
		qi_pan2[y][x] = '#';
		for (int _y = y - 1;_y <= y + 1;_y++) {
			for (int _x = x - 1;_x <= x + 1;_x++) {
				if (_x == x && _y == y)
					continue;
				pan_duan2(_x, _y, qi_pan, qi_pan2);
			}
		}
	}
}

int pan_duan(int x, int y, char qi_pan[][BOARD_N], char qi_pan2[][BOARD_N]) {
	//如果踩雷，game over（原来这里是打印"老弟，回家再练两年"）
	if ('0' == qi_pan2[y][x]) {
		for (int m = 0; m <= 8;m++) {
			for (int n = 0;n <= 8;n++) {
				if ('0' == qi_pan2[m][n]) {
					qi_pan[m][n] = '0';
				}
			}
		}
		g_explodedX = x;
		g_explodedY = y;
		g_state = STATE_LOSE;
		return 0;
	}
	else {
		pan_duan2(x, y, qi_pan, qi_pan2);
		//赢的判定：所有不是雷的格子都翻开就算赢（原来这里是打印"开了？"）
		int fan_kai = 0, lei_shu = 0;
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				if ('*' != qi_pan[i][j]) {
					fan_kai++;
				}
				if ('0' == qi_pan2[i][j]) {
					lei_shu++;
				}
			}
		}
		if (fan_kai == 81 - lei_shu) {
			g_state = STATE_WIN;
			return 0;
		}
		return 1;
	}
}
// ===================== 原控制台版函数结束 =====================

// ---------------- 新局：重置数据 + 布雷 ----------------
void NewGame(void) {
	for (int i = 0; i < BOARD_N; i++) {
		for (int j = 0; j < BOARD_N; j++) {
			qi_pan[i][j] = '*';
			qi_pan2[i][j] = '*';
		}
	}
	g_state = STATE_PLAY;
	g_explodedX = -1;
	g_explodedY = -1;
	g_startTime = 0.0;
	g_endTime = 0.0;
	mai_lei(qi_pan2);
}

// ---------------- 翻开一格（now 由界面用 GetTime() 传入） ----------------
void RevealCell(int cx, int cy, double now) {
	if (g_state != STATE_PLAY) return;
	if (g_startTime == 0.0) g_startTime = now;
	if (pan_duan(cx, cy, qi_pan, qi_pan2) == 0) g_endTime = now;
}

// ---------------- --shot 调试用：构造 lose / win 局面 ----------------
void SetupShotScenario(const char *mode) {
	if (strcmp(mode, "lose") == 0) {
		for (int y = 0; y < BOARD_N; y++) {
			int done = 0;
			for (int x = 0; x < BOARD_N; x++) {
				if (qi_pan2[y][x] == '0') { RevealCell(x, y, 0.0); done = 1; break; }
			}
			if (done) break;
		}
	}
	else if (strcmp(mode, "win") == 0) {
		for (int y = 0; y < BOARD_N; y++) {
			for (int x = 0; x < BOARD_N; x++) {
				if (qi_pan2[y][x] != '0') RevealCell(x, y, 0.0);
			}
		}
	}
}