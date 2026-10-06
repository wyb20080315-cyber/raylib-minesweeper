#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
void print(char qi_pan[][9]) {
	int y = 0;
	printf("  ");
	for (int i = 0;i < 9;i++) {
		printf("%d ", i);
	}
	printf("\n");
	for (y = 0;y < 9;y++) {
		printf("%d ", y);
		for (int x = 0;x < 9;x++) {
			printf("%c ",qi_pan[y][x]);
		}
		printf("\n");
	}
	printf("\n");
}

void mai_lei(char qi_pan2[][9]) {
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
	} while (ci_shu!=11);
}

void pan_duan2(int x, int y, char qi_pan[][9], char qi_pan2[][9]) {
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

int pan_duan(int x, int y, char qi_pan[][9], char qi_pan2[][9]) {
	int lei = 0;
	int if_lei;
	//如果踩雷，game over
	if ('0' == qi_pan2[y][x]) {
		system("cls");
		for (int m = 0; m <= 8;m++) {
			for (int n = 0;n <= 8;n++) {
				if ('0' == qi_pan2[m][n]) {
					qi_pan[m][n] = '0';
				}
			}
		}
		print(qi_pan);
		printf("老弟，回家再练两年");
		return 0;
	}
	else {
		system("cls");
		pan_duan2(x, y, qi_pan, qi_pan2);
		print(qi_pan);
		//赢的判定：所有不是雷的格子都翻开就算赢
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
			printf("开了？\n");
			return 0;
		}
		return 1;
	}
}
int main() {
	int play;
	srand((unsigned int)time(NULL));
	//manu_1();
	scanf("%d", &play);
	do {
		switch (play) {
		case 1: {
			int x = 0, y = 0;

			char qi_pan[9][9];
			for (int i = 0;i < 9;i++) {
				for (int j = 0;j < 9;j++) {
					qi_pan[i][j] = '*';
			}
				}

			char qi_pan2[9][9];
			for (int i = 0;i < 9;i++) {
				for (int j = 0;j < 9;j++) {
					qi_pan2[i][j] = '*';
				}
			}

			int game_over;
			mai_lei(qi_pan2);
			print(qi_pan);
			do {
				//坐标超出 0~8 就提示并重新输入
				do {
					scanf("%d%d", &x, &y);
					if (x < 0 || x>8 || y < 0 || y>8) {
						printf("请重新输入\n");
					}
				} while (x < 0 || x>8 || y < 0 || y>8);
				game_over=pan_duan(x, y,qi_pan,qi_pan2);
			} while (game_over);
			return 0;
		}
		case 0: {
			return 0;
		}
		default:
			printf("请重新输入");
		}
	} while (play != 1 && play != 0);
	return 0;
}