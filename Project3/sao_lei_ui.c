// =====================================================================
//  扫雷 · 界面 sao_lei_ui.c
//  本文件包含全部 raylib 内容：窗口、字体、绘制、鼠标交互、主循环。
//  游戏数据（qi_pan / qi_pan2 / g_state …）定义在 sao_lei_core.c，
//  这里只通过 sao_lei.h 里的 extern 声明使用它们。
//  界面风格：Windows 95 经典扫雷（立体边框 + LED 计数器 + 笑脸按钮）。
// =====================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "raylib.h"
#include "sao_lei.h"

// ---------------- 界面专有数据 ----------------
int qi_flag[BOARD_N][BOARD_N];   // 插旗标记
int g_flagsUsed = 0;             // 已插旗数量
static int g_pressCX = -1;       // 按下鼠标时所在的格子
static int g_pressCY = -1;
static int g_facePressed = 0;    // 是否正按住笑脸

// =====================================================================
//  一、界面常量与配色（Windows 95 经典扫雷风格）
// =====================================================================
#define CELL    32
#define BF      4
#define MARGIN  12
#define HDR_H   60
#define GAP_H   8
#define LED_W   84
#define LED_H   44

#define WIN_W   (MARGIN*2 + BF*2 + CELL*9)
#define WIN_H   (MARGIN*2 + HDR_H + GAP_H + BF*2 + CELL*9 + 32)
#define BOARD_X (MARGIN + BF)
#define BOARD_Y (MARGIN + HDR_H + GAP_H + BF)

#define FACE_SMILE 0
#define FACE_WORRY 1
#define FACE_DEAD  2
#define FACE_COOL  3

static const Color C_FACE    = { 192, 192, 192, 255 };
static const Color C_HILIGHT = { 255, 255, 255, 255 };
static const Color C_SHADOW  = { 128, 128, 128, 255 };
static const Color C_DARK    = { 0, 0, 0, 255 };
static const Color C_RED     = { 255, 0, 0, 255 };
static const Color C_LED     = { 255, 0, 0, 255 };
static const Color C_YELLOW  = { 255, 255, 0, 255 };
static const Color C_HINT    = { 80, 80, 80, 255 };
static const Color C_BAN_BG  = { 24, 24, 24, 235 };
static const Color C_BAN_LOSE= { 255, 120, 120, 255 };
static const Color C_BAN_WIN = { 255, 230, 120, 255 };

static const Color NUM_COLORS[9] = {
	{ 0, 0, 0, 0 },         // 占位
	{ 0, 0, 255, 255 },     // 1 蓝
	{ 0, 128, 0, 255 },     // 2 绿
	{ 255, 0, 0, 255 },     // 3 红
	{ 0, 0, 128, 255 },     // 4 深蓝
	{ 128, 0, 0, 255 },     // 5 深红
	{ 0, 128, 128, 255 },   // 6 青
	{ 0, 0, 0, 255 },       // 7 黑
	{ 128, 128, 128, 255 }, // 8 灰
};

static Font g_fontText;
static Font g_fontBig;
static Font g_fontNum;
static int  g_codepoints[95 + 24];
static int  g_codepointCount = 0;

// 需要显示的中文字形 + 全部 ASCII
static void BuildCodepoints(void) {
	int n = 0;
	for (int c = 32; c <= 126; c++) g_codepoints[n++] = c;
	static const int extra[] = {
		0x5DE6, 0x952E, 0x7FFB, 0x5F00, 0x53F3, 0x63D2, 0x65D7, 0x91CD,
		0x8001, 0x5F1F, 0xFF0C, 0x56DE, 0x5BB6, 0x518D, 0x7EC3, 0x4E24,
		0x5E74, 0x4E86, 0xFF1F
	};
	for (int i = 0; i < (int)(sizeof(extra) / sizeof(extra[0])); i++) {
		g_codepoints[n++] = extra[i];
	}
	g_codepointCount = n;
}

static Font LoadGameFont(int size) {
	if (FileExists("C:/Windows/Fonts/simhei.ttf")) {
		Font f = LoadFontEx("C:/Windows/Fonts/simhei.ttf", size, g_codepoints, g_codepointCount);
		if (f.texture.id != 0) return f;
	}
	return GetFontDefault();
}

static void UnloadGameFont(Font f) {
	if (f.texture.id != GetFontDefault().texture.id) UnloadFont(f);
}

static void SetGameIcon(void) {
	Image img = GenImageColor(32, 32, C_FACE);
	ImageDrawRectangle(&img, 14, 4, 4, 24, C_DARK);
	ImageDrawRectangle(&img, 4, 14, 24, 4, C_DARK);
	ImageDrawCircle(&img, 16, 16, 9, C_DARK);
	ImageDrawRectangle(&img, 11, 11, 4, 4, C_HILIGHT);
	SetWindowIcon(img);
	UnloadImage(img);
}

// =====================================================================
//  二、绘制辅助
// =====================================================================
static void DrawRaisedBox(int x, int y, int w, int h, int b) {
	DrawRectangle(x, y, w, h, C_FACE);
	DrawRectangle(x, y, w, b, C_HILIGHT);
	DrawRectangle(x, y, b, h, C_HILIGHT);
	DrawRectangle(x, y + h - b, w, b, C_SHADOW);
	DrawRectangle(x + w - b, y, b, h, C_SHADOW);
}

static void DrawSunkenBox(int x, int y, int w, int h, int b) {
	DrawRectangle(x, y, w, h, C_FACE);
	DrawRectangle(x, y, w, b, C_SHADOW);
	DrawRectangle(x, y, b, h, C_SHADOW);
	DrawRectangle(x, y + h - b, w, b, C_HILIGHT);
	DrawRectangle(x + w - b, y, b, h, C_HILIGHT);
}

static void DrawLEDDigit(int d, int x, int y, int w, int h) {
	static const unsigned char SEG[11] = {
		0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x40
	};
	if (d < 0 || d > 10) return;
	unsigned char m = SEG[d];
	int t = 5;
	int vh = h / 2 - t / 2 - t;
	if (m & 0x01) DrawRectangle(x + t, y, w - 2 * t, t, C_LED);
	if (m & 0x02) DrawRectangle(x + w - t, y + t, t, vh, C_LED);
	if (m & 0x04) DrawRectangle(x + w - t, y + h / 2 + t / 2, t, vh, C_LED);
	if (m & 0x08) DrawRectangle(x + t, y + h - t, w - 2 * t, t, C_LED);
	if (m & 0x10) DrawRectangle(x, y + h / 2 + t / 2, t, vh, C_LED);
	if (m & 0x20) DrawRectangle(x, y + t, t, vh, C_LED);
	if (m & 0x40) DrawRectangle(x + t, y + h / 2 - t / 2, w - 2 * t, t, C_LED);
}

static void DrawCounter(int value, int x, int y) {
	int v = value;
	if (v < -99) v = -99;
	if (v > 999) v = 999;
	DrawRectangle(x, y, LED_W, LED_H, C_DARK);
	DrawRectangle(x, y, LED_W, 2, C_SHADOW);
	DrawRectangle(x, y, 2, LED_H, C_SHADOW);
	DrawRectangle(x, y + LED_H - 2, LED_W, 2, C_HILIGHT);
	DrawRectangle(x + LED_W - 2, y, 2, LED_H, C_HILIGHT);

	int d0, d1, d2;
	if (v < 0) {
		d0 = 10;
		d1 = (-v) / 10;
		d2 = (-v) % 10;
	}
	else {
		d0 = v / 100;
		d1 = (v / 10) % 10;
		d2 = v % 10;
	}
	DrawLEDDigit(d0, x + 4, y + 4, 24, 36);
	DrawLEDDigit(d1, x + 30, y + 4, 24, 36);
	DrawLEDDigit(d2, x + 56, y + 4, 24, 36);
}

static void DrawMine(int x, int y) {
	int cx = x + CELL / 2;
	int cy = y + CELL / 2;
	DrawRectangle(cx - 2, cy - 13, 4, 26, C_DARK);
	DrawRectangle(cx - 13, cy - 2, 26, 4, C_DARK);
	DrawCircle(cx, cy, 9, C_DARK);
	DrawRectangle(cx - 6, cy - 7, 4, 4, C_HILIGHT);
}

static void DrawFlag(int x, int y) {
	int cx = x + CELL / 2;
	int cy = y + CELL / 2;
	Vector2 v1, v2, v3;
	DrawRectangle(cx + 2, cy - 10, 3, 17, C_DARK);
	DrawRectangle(cx - 8, cy + 7, 15, 4, C_DARK);
	v1.x = (float)(cx + 2);  v1.y = (float)(cy - 11);
	v2.x = (float)(cx - 9);  v2.y = (float)(cy - 6);
	v3.x = (float)(cx + 2);  v3.y = (float)(cy - 1);
	DrawTriangle(v1, v2, v3, C_RED);
}

static void DrawCross(int x, int y) {
	Vector2 a, b;
	a.x = (float)(x + 7);             a.y = (float)(y + 7);
	b.x = (float)(x + CELL - 7);      b.y = (float)(y + CELL - 7);
	DrawLineEx(a, b, 3.0f, C_RED);
	a.x = (float)(x + CELL - 7);      a.y = (float)(y + 7);
	b.x = (float)(x + 7);             b.y = (float)(y + CELL - 7);
	DrawLineEx(a, b, 3.0f, C_RED);
}

static void DrawNumber(int x, int y, int n) {
	char buf[4];
	buf[0] = (char)('0' + n);
	buf[1] = '\0';
	Vector2 sz = MeasureTextEx(g_fontNum, buf, 24, 0);
	Vector2 p;
	p.x = x + (CELL - sz.x) / 2.0f;
	p.y = y + (CELL - sz.y) / 2.0f - 1.0f;
	DrawTextEx(g_fontNum, buf, p, 24, 0, NUM_COLORS[n]);
}
static void DrawFaceButton(int x, int y, int size, int mood) {
	if (mood == FACE_WORRY) DrawSunkenBox(x, y, size, size, 3);
	else DrawRaisedBox(x, y, size, size, 3);

	int cx = x + size / 2;
	int cy = y + size / 2;
	DrawCircle(cx, cy, 15.5f, C_YELLOW);

	if (mood == FACE_DEAD) {
		Vector2 a, b;
		a.x = (float)(cx - 9); a.y = (float)(cy - 8);  b.x = (float)(cx - 4); b.y = (float)(cy - 3);
		DrawLineEx(a, b, 2.0f, C_DARK);
		a.x = (float)(cx - 4); a.y = (float)(cy - 8);  b.x = (float)(cx - 9); b.y = (float)(cy - 3);
		DrawLineEx(a, b, 2.0f, C_DARK);
		a.x = (float)(cx + 4); a.y = (float)(cy - 8);  b.x = (float)(cx + 9); b.y = (float)(cy - 3);
		DrawLineEx(a, b, 2.0f, C_DARK);
		a.x = (float)(cx + 9); a.y = (float)(cy - 8);  b.x = (float)(cx + 4); b.y = (float)(cy - 3);
		DrawLineEx(a, b, 2.0f, C_DARK);
		a.x = (float)cx; a.y = (float)(cy + 12);
		DrawRing(a, 6.0f, 8.0f, 200, 340, 16, C_DARK);
	}
	else if (mood == FACE_COOL) {
		DrawRectangle(cx - 11, cy - 7, 8, 6, C_DARK);
		DrawRectangle(cx + 3, cy - 7, 8, 6, C_DARK);
		DrawRectangle(cx - 3, cy - 6, 6, 2, C_DARK);
		Vector2 c;
		c.x = (float)cx; c.y = (float)(cy + 1);
		DrawRing(c, 6.0f, 8.0f, 30, 150, 16, C_DARK);
	}
	else {
		DrawRectangle(cx - 7, cy - 7, 3, 5, C_DARK);
		DrawRectangle(cx + 4, cy - 7, 3, 5, C_DARK);
		Vector2 c;
		if (mood == FACE_WORRY) {
			c.x = (float)cx; c.y = (float)(cy + 7);
			DrawRing(c, 2.0f, 4.0f, 0, 360, 16, C_DARK);
		}
		else {
			c.x = (float)cx; c.y = (float)(cy + 1);
			DrawRing(c, 6.0f, 8.0f, 30, 150, 16, C_DARK);
		}
	}
}

static void DrawCell(int cx, int cy) {
	int x = BOARD_X + cx * CELL;
	int y = BOARD_Y + cy * CELL;
	char c = qi_pan[cy][cx];
	int revealed = (c != '*');

	if (!revealed) {
		DrawRectangle(x, y, CELL, CELL, C_FACE);
		DrawRectangle(x, y, CELL, 3, C_HILIGHT);
		DrawRectangle(x, y, 3, CELL, C_HILIGHT);
		DrawRectangle(x, y + CELL - 3, CELL, 3, C_SHADOW);
		DrawRectangle(x + CELL - 3, y, 3, CELL, C_SHADOW);
	}
	else {
		DrawRectangle(x, y, CELL, CELL, C_FACE);
		DrawRectangle(x, y, CELL, 1, C_SHADOW);
		DrawRectangle(x, y, 1, CELL, C_SHADOW);
	}

	// 失败后：翻开所有雷、纠正插错的旗
	if (g_state == STATE_LOSE) {
		if (qi_pan2[cy][cx] == '0') {
			if (qi_flag[cy][cx]) { DrawFlag(x, y); return; }
			if (cx == g_explodedX && cy == g_explodedY)
				DrawRectangle(x, y, CELL, CELL, C_RED);
			DrawMine(x, y);
			return;
		}
		if (qi_flag[cy][cx]) {
			DrawMine(x, y);
			DrawCross(x, y);
			return;
		}
	}
	// 胜利后：给所有雷自动插旗
	if (g_state == STATE_WIN && qi_pan2[cy][cx] == '0') { DrawFlag(x, y); return; }

	if (qi_flag[cy][cx] && !revealed) { DrawFlag(x, y); return; }
	if (!revealed) return;
	if (c >= '1' && c <= '8') { DrawNumber(x, y, c - '0'); return; }
	if (c == '0') DrawMine(x, y);
}

static void DrawBanner(void) {
	const char *msg;
	Color col;
	if (g_state == STATE_LOSE) { msg = "老弟，回家再练两年"; col = C_BAN_LOSE; }
	else { msg = "开了？"; col = C_BAN_WIN; }

	Vector2 sz = MeasureTextEx(g_fontBig, msg, 26, 0);
	int bw = (int)sz.x + 32;
	int bh = (int)sz.y + 20;
	if (bw > WIN_W - 16) bw = WIN_W - 16;
	int bx = (WIN_W - bw) / 2;
	int by = BOARD_Y + (CELL * 9 - bh) / 2;

	DrawRectangle(bx, by, bw, bh, C_BAN_BG);
	Rectangle rr;
	rr.x = (float)bx; rr.y = (float)by;
	rr.width = (float)bw; rr.height = (float)bh;
	DrawRectangleLinesEx(rr, 2.0f, C_HILIGHT);

	Vector2 tp;
	tp.x = bx + (bw - sz.x) / 2.0f;
	tp.y = by + (bh - sz.y) / 2.0f;
	DrawTextEx(g_fontBig, msg, tp, 26, 0, col);
}

static void DrawGame(void) {
	// 窗口底 + 立体外框
	DrawRectangle(0, 0, WIN_W, WIN_H, C_FACE);
	DrawRectangle(0, 0, WIN_W, 2, C_HILIGHT);
	DrawRectangle(0, 0, 2, WIN_H, C_HILIGHT);
	DrawRectangle(0, WIN_H - 2, WIN_W, 2, C_SHADOW);
	DrawRectangle(WIN_W - 2, 0, 2, WIN_H, C_SHADOW);

	// 顶部面板（凹槽）：计数器 + 笑脸
	DrawSunkenBox(MARGIN, MARGIN, WIN_W - 2 * MARGIN, HDR_H, BF);

	int ledY = MARGIN + (HDR_H - LED_H) / 2;
	DrawCounter(MINE_TOTAL - g_flagsUsed, MARGIN + BF + 8, ledY);

	int secs = 0;
	if (g_startTime > 0.0) {
		double tEnd = (g_state == STATE_PLAY) ? GetTime() : g_endTime;
		secs = (int)(tEnd - g_startTime);
		if (secs < 0) secs = 0;
		if (secs > 999) secs = 999;
	}
	DrawCounter(secs, WIN_W - MARGIN - BF - 8 - LED_W, ledY);

	int mood = FACE_SMILE;
	if (g_state == STATE_LOSE) mood = FACE_DEAD;
	else if (g_state == STATE_WIN) mood = FACE_COOL;
	else if (g_facePressed || g_pressCX >= 0) mood = FACE_WORRY;
	DrawFaceButton(WIN_W / 2 - 24, MARGIN + (HDR_H - 48) / 2, 48, mood);

	// 雷区面板（凹槽）+ 81 个格子
	DrawSunkenBox(MARGIN, MARGIN + HDR_H + GAP_H, BF * 2 + CELL * 9, BF * 2 + CELL * 9, BF);
	for (int cy = 0; cy < 9; cy++) {
		for (int cx = 0; cx < 9; cx++) {
			DrawCell(cx, cy);
		}
	}

	// 底部操作提示
	const char *hint = "左键翻开    右键插旗    R 键重开";
	Vector2 hs = MeasureTextEx(g_fontText, hint, 20, 0);
	Vector2 hp;
	hp.x = (WIN_W - hs.x) / 2.0f;
	hp.y = MARGIN + HDR_H + GAP_H + BF * 2 + CELL * 9 + 6.0f;
	DrawTextEx(g_fontText, hint, hp, 20, 0, C_HINT);

	// 结算横幅
	if (g_state != STATE_PLAY) DrawBanner();
}

// =====================================================================
//  三、游戏流程（界面侧）
// =====================================================================
// 开新局：先清掉界面自己的状态（旗子、按下状态），再让核心逻辑重置棋盘
static void NewUiGame(void) {
	for (int i = 0; i < BOARD_N; i++) {
		for (int j = 0; j < BOARD_N; j++) {
			qi_flag[i][j] = 0;
		}
	}
	g_flagsUsed = 0;
	g_pressCX = -1;
	g_pressCY = -1;
	g_facePressed = 0;
	NewGame();          // ← 核心逻辑（sao_lei_core.c）
}

static void ToggleFlag(int cx, int cy) {
	if (g_state != STATE_PLAY) return;
	if (qi_pan[cy][cx] != '*') return;
	qi_flag[cy][cx] = !qi_flag[cy][cx];
	g_flagsUsed += qi_flag[cy][cx] ? 1 : -1;
}

static int MouseToCell(Vector2 m, int *cx, int *cy) {
	if (m.x < (float)BOARD_X || m.y < (float)BOARD_Y) return 0;
	int x = (int)((m.x - BOARD_X) / CELL);
	int y = (int)((m.y - BOARD_Y) / CELL);
	if (x < 0 || x > 8 || y < 0 || y > 8) return 0;
	*cx = x;
	*cy = y;
	return 1;
}

static void UpdateGame(void) {
	Vector2 m = GetMousePosition();
	Rectangle face;
	face.x = (float)(WIN_W / 2 - 24);
	face.y = (float)(MARGIN + (HDR_H - 48) / 2);
	face.width = 48.0f;
	face.height = 48.0f;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		if (CheckCollisionPointRec(m, face)) {
			g_facePressed = 1;
		}
		else {
			int cx, cy;
			if (g_state == STATE_PLAY && MouseToCell(m, &cx, &cy)) {
				g_pressCX = cx;
				g_pressCY = cy;
			}
		}
	}

	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		if (g_facePressed) {
			if (CheckCollisionPointRec(m, face)) NewUiGame();
			g_facePressed = 0;
		}
		else if (g_pressCX >= 0) {
			int cx, cy;
			if (MouseToCell(m, &cx, &cy) && cx == g_pressCX && cy == g_pressCY) {
				if (!qi_flag[cy][cx] && qi_pan[cy][cx] == '*') RevealCell(cx, cy, GetTime());
			}
			g_pressCX = -1;
			g_pressCY = -1;
		}
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		int cx, cy;
		if (MouseToCell(m, &cx, &cy)) ToggleFlag(cx, cy);
	}

	if (IsKeyPressed(KEY_R)) NewUiGame();
}

// =====================================================================
//  四、界面入口：建窗口 + 主循环
// =====================================================================
int UiRun(int shot, const char *shotFile) {
	BuildCodepoints();

	InitWindow(WIN_W, WIN_H, "扫雷 - Windows 经典版");
	if (shot) SetWindowState(FLAG_WINDOW_HIDDEN);
	SetTargetFPS(60);
	SetGameIcon();

	g_fontText = LoadGameFont(20);
	g_fontBig = LoadGameFont(26);
	g_fontNum = LoadGameFont(24);

	{
		int mw = GetMonitorWidth(0);
		int mh = GetMonitorHeight(0);
		SetWindowPosition((mw - WIN_W) / 2, (mh - WIN_H) / 2 - 40);
	}

	int frames = 0;
	while (!WindowShouldClose()) {
		UpdateGame();

		BeginDrawing();
		DrawGame();
		if (shot && frames >= 2) TakeScreenshot(shotFile);
		EndDrawing();

		frames++;
		if (shot && frames >= 3) break;
	}

	UnloadGameFont(g_fontText);
	UnloadGameFont(g_fontBig);
	UnloadGameFont(g_fontNum);
	CloseWindow();
	return 0;
}