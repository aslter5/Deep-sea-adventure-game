// Fish Game - 完整可编译版
// 引入必要的头文件：raylib图形库、标准库、数学库、时间库、字符串库、文件库
#include "raylib.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

// ===================== 游戏常量设置 =====================
#define SCREEN_WIDTH 1600          // 屏幕宽度
#define SCREEN_HEIGHT 900         // 屏幕高度
#define MAX_SMALL_FISH 100        // 小鱼最大数量
#define MAX_ENEMY_FISH 30         // 敌人鱼最大数量
#define INITIAL_SMALL_FISH 30     // 初始小鱼数量
#define INITIAL_ENEMY_FISH 8      // 初始敌人鱼数量
#define INITIAL_PLAYER_SIZE 20.0f // 玩家初始大小
#define WIN_SIZE 68.0f           // 胜利所需大小
#define MAX_LIVES 3               // 最大生命值
#define INVINCIBLE_TIME 3.0f      // 无敌时间(秒)
#define EAT_THRESHOLD 1.05f       // 吞噬判定阈值（自身大小需大于目标1.05倍）
#define SMALL_FISH_FLEE_DISTANCE 150.0f // 小鱼逃跑距离
#define ENEMY_CHASE_DISTANCE 300.0f     // 敌人追击距离
#define WORLD_WIDTH 3000          // 游戏世界宽度
#define WORLD_HEIGHT 2000         // 游戏世界高度
#define FISH_SPAWN_INTERVAL 8.0f  // 额外鱼生成间隔(秒)
#define ENEMY_SPEED_INCREASE 0.5f // 敌人速度增长值
#define MAX_ENEMY_SPEED 4.0f      // 敌人最大速度值
#define ENEMY_SIZE_INCREASE 2.0f  // 敌人大小增长
#define MAX_ENEMY_SIZE 300.0f     // 敌人最大大小
#define SMALL_FISH_SPAWN_COUNT 3  // 每次生成小鱼数量
#define ENEMY_FISH_SPAWN_COUNT 2  // 每次生成敌人鱼数量
#define HIGH_SCORE_FILE "fish_game_highscore.dat" // 最高分保存文件

// ===================== 枚举类型定义 =====================
// 游戏状态枚举
typedef enum {
	GAME_MENU = 0,         // 主菜单
	GAME_PLAYING = 1,      // 游戏进行中
	GAME_OVER = 2,         // 游戏结束
	GAME_CERTIFICATE = 3,  // 胜利证书
	GAME_ACHIEVEMENT = 4   // 成就解锁
} GameState;

// 游戏模式枚举
typedef enum {
	MODE_ENDLESS = 0,      // 无尽模式
	MODE_LEVELS = 1        // 关卡模式
} GameMode;

// 成就类型枚举
typedef enum {
	ACHIEVEMENT_NONE = 0,          // 无成就
	ACHIEVEMENT_BREAK_1000 = 1,    // 分数突破1000
	ACHIEVEMENT_BREAK_2000 = 2,    // 分数突破2000
	ACHIEVEMENT_COMPLETE_LEVELS = 3 // 完成所有关卡
} AchievementType;

// ===================== 结构体定义 =====================
// 鱼的结构体（包含玩家、小鱼、敌人鱼的所有属性）
typedef struct {
	Vector2 position;      // 位置坐标
	Vector2 velocity;      // 移动速度向量
	float size;            // 大小
	Color color;           // 身体颜色
	Color eyeColor;        // 眼睛颜色
	Color finColor;        // 鱼鳍颜色
	int type;              // 类型：0-玩家 1-小鱼 2-敌人鱼
	int active;            // 是否激活（存活）
	float rotation;        // 旋转角度
	float wiggleAngle;     // 摆动角度（动画效果）
	float wiggleSpeed;     // 摆动速度
	float currentSpeed;    // 当前移动速度
	int value;             // 被吃掉后获得的分数
	float aiTimer;         // AI行为计时器
	float targetX;         // AI目标X坐标
	float targetY;         // AI目标Y坐标
	int health;            // 生命值
	int maxHealth;         // 最大生命值
} Fish;

// ===================== 全局变量声明 =====================
Fish player;                       // 玩家鱼
Fish smallFish[MAX_SMALL_FISH];    // 小鱼数组
Fish enemyFish[MAX_ENEMY_FISH];    // 敌人鱼数组
Fish boss;                         // BOSS鱼

int score = 0;                     // 当前分数
int highScoreEndless = 0;          // 无尽模式最高分
int highScoreLevels = 0;           // 关卡模式最高分
int lives = MAX_LIVES;             // 当前生命值
GameState gameState = GAME_MENU;   // 当前游戏状态
GameMode currentMode = MODE_ENDLESS; // 当前游戏模式
int isInvincible = 0;              // 是否无敌
float invincibleTimer = 0.0f;      // 无敌时间计时器
float gameTime = 0.0f;             // 游戏进行时间
int level = 1;                     // 当前关卡
int fishEaten = 0;                 // 吃掉的鱼数量
float cameraX = SCREEN_WIDTH / 2.0f; // 相机X坐标
float cameraY = SCREEN_HEIGHT / 2.0f;// 相机Y坐标
float cameraZoom = 1.0f;           // 相机缩放比例
int respawnTimer = 0;              // 鱼重生计时器
int activeSmallFishCount = 0;      // 活跃小鱼数量
int activeEnemyFishCount = 0;      // 活跃敌人鱼数量
int bossActive = 0;                // BOSS是否激活
float bossSpawnTimer = 5.0f;       // BOSS生成计时器
int levelCompleted = 0;            // 关卡是否完成
int gameWon = 0;                   // 是否赢得游戏
float fishSpawnTimer = 0.0f;       // 额外鱼生成计时器
float enemyBaseSpeed = 1.2f;       // 敌人基础速度
int spawnMultiplier = 1;           // 生成倍率
float enemyBaseSize = 30.0f;       // 敌人基础大小
int maxLevel = 3;                  // 最大关卡数
int hasWonCertificate = 0;         // 是否获得胜利证书
int hasAchievement1000 = 0;        // 是否解锁1000分成就
int hasAchievement2000 = 0;        // 是否解锁2000分成就
AchievementType currentAchievement = ACHIEVEMENT_NONE; // 当前解锁成就
float achievementShowTime = 0.0f;  // 成就显示时间
float certificateShowTime = 0.0f;  // 证书显示时间
int isMuted = 0;                   // 是否静音
RenderTexture2D gameTexture;       // 游戏渲染纹理（解决闪烁）

// ===================== 函数声明 =====================
void InitGame(GameMode mode);              // 初始化游戏
float FishDistance(float x1, float y1, float x2, float y2); // 计算鱼之间的距离
void DrawFish(Fish fish);                 // 绘制鱼
void UpdatePlayer(void);                  // 更新玩家状态
void CheckCollisions(void);               // 检查碰撞
void RespawnFish(void);                   // 重生鱼
void SpawnAdditionalFish(void);           // 生成额外的鱼
void DrawSeamlessBackground(void);        // 绘制无缝背景
void DrawUI(void);                        // 绘制UI界面
void DrawMainMenu(void);                  // 绘制主菜单
void DrawGameOver(void);                  // 绘制游戏结束界面
void SpawnSmallFish(void);                // 生成小鱼
void SpawnEnemyFish(void);                // 生成敌人鱼
void LoadHighScores(void);                // 加载最高分
void SaveHighScores(void);                // 保存最高分
void UpdateHighScore(void);               // 更新最高分
void CheckAchievements(void);             // 检查成就
void DrawCertificate(void);               // 绘制胜利证书
void DrawAchievement(void);               // 绘制成就

// ===================== 工具函数实现 =====================
// 浮点数范围限制函数
float ClampValue(float value, float min, float max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

// 整数范围限制函数
int ClampInt(int value, int min, int max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

// ===================== 分数相关函数 =====================
// 加载最高分
void LoadHighScores(void) {
	FILE* file = fopen(HIGH_SCORE_FILE, "rb"); // 以二进制读模式打开文件
	if (file != NULL) {
		fread(&highScoreEndless, sizeof(int), 1, file); // 读取无尽模式最高分
		fread(&highScoreLevels, sizeof(int), 1, file);  // 读取关卡模式最高分
		fclose(file); // 关闭文件
	}
}

// 保存最高分
void SaveHighScores(void) {
	FILE* file = fopen(HIGH_SCORE_FILE, "wb"); // 以二进制写模式打开文件
	if (file != NULL) {
		fwrite(&highScoreEndless, sizeof(int), 1, file); // 写入无尽模式最高分
		fwrite(&highScoreLevels, sizeof(int), 1, file);  // 写入关卡模式最高分
		fclose(file); // 关闭文件
	}
}

// 更新最高分
void UpdateHighScore(void) {
	if (currentMode == MODE_ENDLESS) {
		if (score > highScoreEndless) { // 超过无尽模式最高分
			highScoreEndless = score;
			SaveHighScores(); // 保存新记录
		}
	} else {
		if (score > highScoreLevels) { // 超过关卡模式最高分
			highScoreLevels = score;
			SaveHighScores(); // 保存新记录
		}
	}
}

// ===================== 成就相关函数 =====================
// 检查成就解锁条件
void CheckAchievements(void) {
	if (currentMode == MODE_ENDLESS) {
		// 2000分成就
		if (score >= 2000 && !hasAchievement2000) {
			hasAchievement2000 = 1;
			currentAchievement = ACHIEVEMENT_BREAK_2000;
			achievementShowTime = 3.0f;
			gameState = GAME_ACHIEVEMENT;
		}
		// 1000分成就
		else if (score >= 1000 && !hasAchievement1000) {
			hasAchievement1000 = 1;
			currentAchievement = ACHIEVEMENT_BREAK_1000;
			achievementShowTime = 3.0f;
			gameState = GAME_ACHIEVEMENT;
		}
	} else if (currentMode == MODE_LEVELS) {
		// 完成所有关卡成就
		if (level > maxLevel && !hasWonCertificate) {
			hasWonCertificate = 1;
			certificateShowTime = 5.0f;
			gameState = GAME_CERTIFICATE;
		}
	}
}

// ===================== 游戏初始化函数 =====================
// 初始化游戏（根据模式）
void InitGame(GameMode mode) {
	srand((unsigned int)time(NULL)); // 初始化随机数种子
	
	currentMode = mode;
	level = 1;
	activeSmallFishCount = 0;
	activeEnemyFishCount = 0;
	fishSpawnTimer = 0.0f;
	enemyBaseSpeed = 1.2f;
	enemyBaseSize = 30.0f;
	spawnMultiplier = 1;
	
	// 初始化玩家鱼属性
	player.position = (Vector2){SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f};
	player.velocity = (Vector2){0.0f, 0.0f};
	player.size = INITIAL_PLAYER_SIZE;
	player.color = (Color){0, 185, 235, 255}; // 玩家主色（浅蓝色）
	player.eyeColor = BLACK;                  // 眼睛黑色
	player.finColor = (Color){0, 155, 210, 255}; // 鱼鳍颜色
	player.type = 0;                          // 类型：玩家
	player.active = 1;                        // 激活状态
	player.rotation = 0.0f;                   // 初始旋转角度
	player.wiggleAngle = 0.0f;                // 初始摆动角度
	player.wiggleSpeed = 5.0f;                // 摆动速度
	player.currentSpeed = 5.0f;               // 移动速度
	player.value = 0;                         // 分数值（玩家无）
	player.aiTimer = 0.0f;                    // AI计时器
	player.targetX = player.position.x;       // AI目标X
	player.targetY = player.position.y;       // AI目标Y
	player.health = 1;                        // 生命值
	player.maxHealth = 1;                     // 最大生命值
	
	// 重置所有小鱼为未激活状态
	for (int i = 0; i < MAX_SMALL_FISH; i++) {
		smallFish[i].active = 0;
	}
	// 重置所有敌人鱼为未激活状态
	for (int i = 0; i < MAX_ENEMY_FISH; i++) {
		enemyFish[i].active = 0;
	}
	
	// 生成初始小鱼
	for (int i = 0; i < INITIAL_SMALL_FISH; i++) {
		if (i < MAX_SMALL_FISH) {
			SpawnSmallFish();
		}
	}
	
	// 生成初始敌人鱼
	for (int i = 0; i < INITIAL_ENEMY_FISH; i++) {
		if (i < MAX_ENEMY_FISH) {
			SpawnEnemyFish();
		}
	}
	
	// 初始化BOSS为未激活状态
	boss.active = 0;
	
	// 重置游戏状态变量
	score = 0;
	lives = MAX_LIVES;
	isInvincible = 1;         // 初始无敌
	invincibleTimer = 0.0f;   // 无敌计时器
	gameTime = 0.0f;          // 游戏时间
	fishEaten = 0;            // 吃掉的鱼数量
	cameraX = player.position.x; // 相机初始位置
	cameraY = player.position.y;
	cameraZoom = 1.0f;        // 相机缩放
	respawnTimer = 0;         // 重生计时器
	bossActive = 0;           // BOSS未激活
	bossSpawnTimer = 5.0f;    // BOSS生成计时器
	levelCompleted = 0;       // 关卡未完成
	gameWon = 0;              // 未获胜
	hasWonCertificate = 0;    // 未获得证书
	currentAchievement = ACHIEVEMENT_NONE; // 无成就
	
	if (currentMode == MODE_LEVELS) {
		bossSpawnTimer = 5.0f; // 关卡模式BOSS生成时间
	}
}

// ===================== 鱼生成函数 =====================
// 生成小鱼
void SpawnSmallFish(void) {
	// 遍历小鱼数组找空位
	for (int i = 0; i < MAX_SMALL_FISH; i++) {
		if (!smallFish[i].active) {
			float x, y;
			int attempts = 0;
			// 生成远离玩家的位置（最多10次尝试）
			do {
				x = (float)(rand() % WORLD_WIDTH);
				y = (float)(rand() % WORLD_HEIGHT);
				attempts++;
			} while (FishDistance(x, y, player.position.x, player.position.y) < 200.0f && attempts < 10);
			
			// 设置小鱼属性
			smallFish[i].position = (Vector2){x, y};
			// 随机初始速度
			smallFish[i].velocity = (Vector2){
				(float)(rand() % 81 - 40) / 35.0f,
				(float)(rand() % 81 - 40) / 35.0f
			};
			smallFish[i].size = 8.0f + (float)(rand() % 12); // 大小8-20
			
			// 随机颜色
			int colorChoice = rand() % 10;
			switch (colorChoice) {
				case 0: smallFish[i].color = (Color){100, 245, 130, 255}; break; // 浅绿色
				case 1: smallFish[i].color = (Color){255, 195, 75, 255}; break;  // 浅黄色
				case 2: smallFish[i].color = (Color){175, 115, 235, 255}; break; // 浅紫色
				case 3: smallFish[i].color = (Color){245, 125, 95, 255}; break;  // 浅红色
				case 4: smallFish[i].color = (Color){100, 200, 255, 255}; break; // 浅蓝色
				case 5: smallFish[i].color = (Color){255, 105, 180, 255}; break; // 浅粉色
				case 6: smallFish[i].color = (Color){255, 255, 100, 255}; break; // 亮黄色
				case 7: smallFish[i].color = (Color){150, 255, 150, 255}; break; // 嫩绿色
				case 8: smallFish[i].color = (Color){255, 150, 150, 255}; break; // 浅粉色
				case 9: smallFish[i].color = (Color){200, 150, 255, 255}; break; // 浅紫色
			}
			
			smallFish[i].eyeColor = BLACK; // 眼睛黑色
			// 鱼鳍颜色（比身体亮一点）
			smallFish[i].finColor = (Color){
				(unsigned char)(smallFish[i].color.r + 30),
				(unsigned char)(smallFish[i].color.g + 30),
				(unsigned char)(smallFish[i].color.b + 30),
				255
			};
			smallFish[i].type = 1;                  // 类型：小鱼
			smallFish[i].active = 1;                // 激活
			smallFish[i].rotation = 0.0f;           // 初始旋转
			smallFish[i].wiggleAngle = (float)(rand() % 360) * 3.14159265f / 180.0f; // 随机初始摆动角度
			smallFish[i].wiggleSpeed = 3.0f + (float)(rand() % 100) / 100.0f; // 摆动速度
			smallFish[i].currentSpeed = 1.5f + (float)(rand() % 100) / 100.0f; // 移动速度
			smallFish[i].value = 5;                 // 分数值5
			smallFish[i].aiTimer = 0.0f;            // AI计时器
			smallFish[i].targetX = (float)(rand() % WORLD_WIDTH); // 随机目标X
			smallFish[i].targetY = (float)(rand() % WORLD_HEIGHT); // 随机目标Y
			smallFish[i].health = 1;                // 生命值
			smallFish[i].maxHealth = 1;             // 最大生命值
			activeSmallFishCount++;                 // 活跃小鱼数+1
			break;
		}
	}
}

// 生成敌人鱼
void SpawnEnemyFish(void) {
	// 遍历敌人鱼数组找空位
	for (int i = 0; i < MAX_ENEMY_FISH; i++) {
		if (!enemyFish[i].active) {
			float x, y;
			int attempts = 0;
			// 生成远离玩家的位置（最多10次尝试）
			do {
				x = (float)(rand() % WORLD_WIDTH);
				y = (float)(rand() % WORLD_HEIGHT);
				attempts++;
			} while (FishDistance(x, y, player.position.x, player.position.y) < 300.0f && attempts < 10);
			
			// 设置敌人鱼属性
			enemyFish[i].position = (Vector2){x, y};
			
			float speedMultiplier = 1.0f + spawnMultiplier * 0.1f; // 速度倍率
			// 随机初始速度
			enemyFish[i].velocity = (Vector2){
				(float)(rand() % 61 - 30) / 45.0f * speedMultiplier,
				(float)(rand() % 61 - 30) / 45.0f * speedMultiplier
			};
			
			// 敌人大小（基础大小+随机+倍率）
			float enemySize = enemyBaseSize + (float)(rand() % 20) + spawnMultiplier * ENEMY_SIZE_INCREASE;
			if (enemySize > MAX_ENEMY_SIZE) enemySize = MAX_ENEMY_SIZE; // 限制最大大小
			
			enemyFish[i].size = enemySize;
			
			// 随机颜色类型
			int colorType = rand() % 4;
			switch (colorType) {
				case 0: enemyFish[i].color = (Color){235, 72, 68, 255}; break;  // 红色
				case 1: enemyFish[i].color = (Color){185, 48, 44, 255}; break;  // 深红色
				case 2: enemyFish[i].color = (Color){150, 50, 200, 255}; break; // 紫色
				case 3: enemyFish[i].color = (Color){255, 100, 50, 255}; break; // 橙色
			}
			
			enemyFish[i].eyeColor = (Color){255, 248, 192, 255}; // 眼睛（米白色）
			// 鱼鳍颜色（比身体亮一点）
			enemyFish[i].finColor = (Color){
				(unsigned char)(enemyFish[i].color.r + 40),
				(unsigned char)(enemyFish[i].color.g + 40),
				(unsigned char)(enemyFish[i].color.b + 40),
				255
			};
			enemyFish[i].type = 2;                  // 类型：敌人鱼
			enemyFish[i].active = 1;                // 激活
			enemyFish[i].rotation = 0.0f;           // 初始旋转
			enemyFish[i].wiggleAngle = (float)(rand() % 360) * 3.14159265f / 180.0f; // 随机初始摆动角度
			enemyFish[i].wiggleSpeed = 1.5f;        // 摆动速度
			// 移动速度（基础+倍率，限制最大值）
			float speed = enemyBaseSpeed + spawnMultiplier * ENEMY_SPEED_INCREASE;
			if (speed > MAX_ENEMY_SPEED) speed = MAX_ENEMY_SPEED;
			enemyFish[i].currentSpeed = speed;
			
			enemyFish[i].value = 15 + spawnMultiplier * 2; // 分数值
			enemyFish[i].aiTimer = 0.0f;                  // AI计时器
			enemyFish[i].targetX = (float)(rand() % WORLD_WIDTH); // 随机目标X
			enemyFish[i].targetY = (float)(rand() % WORLD_HEIGHT); // 随机目标Y
			enemyFish[i].health = 1;                      // 生命值
			enemyFish[i].maxHealth = 1;                   // 最大生命值
			activeEnemyFishCount++;                       // 活跃敌人鱼数+1
			break;
		}
	}
}

// 生成额外的鱼（游戏进行中）
void SpawnAdditionalFish(void) {
	if (gameState != GAME_PLAYING) return; // 非游戏中不执行
	
	fishSpawnTimer += GetFrameTime(); // 累加生成计时器
	
	// 达到生成间隔
	if (fishSpawnTimer >= FISH_SPAWN_INTERVAL) {
		fishSpawnTimer = 0.0f;
		spawnMultiplier++; // 生成倍率+1
		
		// 增加敌人基础速度（限制最大值）
		enemyBaseSpeed += ENEMY_SPEED_INCREASE;
		if (enemyBaseSpeed > MAX_ENEMY_SPEED) enemyBaseSpeed = MAX_ENEMY_SPEED;
		
		// 增加敌人基础大小（限制最大值）
		enemyBaseSize += ENEMY_SIZE_INCREASE;
		if (enemyBaseSize > MAX_ENEMY_SIZE) enemyBaseSize = MAX_ENEMY_SIZE;
		
		// 更新现有敌人鱼的速度和大小
		for (int i = 0; i < MAX_ENEMY_FISH; i++) {
			if (enemyFish[i].active) {
				enemyFish[i].currentSpeed = enemyBaseSpeed;
				enemyFish[i].size = enemyBaseSize + (float)(rand() % 20);
				if (enemyFish[i].size > MAX_ENEMY_SIZE) enemyFish[i].size = MAX_ENEMY_SIZE;
			}
		}
		
		// 生成新的小鱼
		for (int j = 0; j < SMALL_FISH_SPAWN_COUNT; j++) {
			if (activeSmallFishCount < MAX_SMALL_FISH) {
				SpawnSmallFish();
			}
		}
		
		// 生成新的敌人鱼
		for (int j = 0; j < ENEMY_FISH_SPAWN_COUNT; j++) {
			if (activeEnemyFishCount < MAX_ENEMY_FISH) {
				SpawnEnemyFish();
			}
		}
		
		CheckAchievements(); // 检查成就
	}
}

// ===================== 辅助函数 =====================
// 计算两条鱼之间的距离
float FishDistance(float x1, float y1, float x2, float y2) {
	float dx = x2 - x1; // X轴差值
	float dy = y2 - y1; // Y轴差值
	return sqrtf(dx*dx + dy*dy); // 欧几里得距离
}

// ===================== 绘制函数 =====================
// 绘制鱼（包含身体、鱼鳍、眼睛等细节）
void DrawFish(Fish fish) {
	if (!fish.active) return; // 未激活的鱼不绘制
	
	// 更新摆动角度（动画效果）
	fish.wiggleAngle += GetFrameTime() * fish.wiggleSpeed;
	float wiggle = sinf(fish.wiggleAngle) * fish.size * 0.1f; // 摆动幅度
	
	// 鱼身体尺寸
	float bodyLength = fish.size * 1.4f;
	float bodyWidth = fish.size * 0.6f;
	
	// 计算旋转角度（根据移动方向）
	float rotation = (fish.velocity.x != 0.0f || fish.velocity.y != 0.0f) 
	? atan2f(fish.velocity.y, fish.velocity.x) 
	: fish.rotation;
	
	Vector2 center = fish.position; // 鱼的中心位置
	
	// 绘制鱼身体（椭圆）
	DrawEllipse((int)center.x, (int)center.y, bodyLength/2, (bodyWidth+wiggle)/2, fish.color);
	
	// 计算鱼尾坐标
	Vector2 tail = (Vector2){
		center.x - cosf(rotation) * bodyLength * 0.4f,
		center.y - sinf(rotation) * bodyLength * 0.4f
	};
	Vector2 tailLeft = (Vector2){
		tail.x - sinf(rotation) * bodyWidth * 0.6f,
		tail.y + cosf(rotation) * bodyWidth * 0.6f
	};
	Vector2 tailRight = (Vector2){
		tail.x + sinf(rotation) * bodyWidth * 0.6f,
		tail.y - cosf(rotation) * bodyWidth * 0.6f
	};
	Vector2 tailTip = (Vector2){
		tail.x - cosf(rotation) * bodyLength * 0.3f,
		tail.y - sinf(rotation) * bodyLength * 0.3f
	};
	
	// 绘制鱼尾（两个三角形）
	DrawTriangle(tail, tailLeft, tailTip, fish.finColor);
	DrawTriangle(tail, tailRight, tailTip, fish.finColor);
	
	// 敌人鱼绘制背鳍
	if (fish.type == 2) {
		Vector2 dorsal1 = (Vector2){
			center.x - sinf(rotation) * bodyWidth * 0.4f + cosf(rotation) * bodyLength * 0.1f,
			center.y + cosf(rotation) * bodyWidth * 0.4f + sinf(rotation) * bodyLength * 0.1f
		};
		Vector2 dorsal2 = (Vector2){
			center.x - sinf(rotation) * bodyWidth * 0.6f - cosf(rotation) * bodyLength * 0.2f,
			center.y + cosf(rotation) * bodyWidth * 0.6f - sinf(rotation) * bodyLength * 0.2f
		};
		Vector2 dorsal3 = (Vector2){
			center.x - sinf(rotation) * bodyWidth * 0.4f - cosf(rotation) * bodyLength * 0.4f,
			center.y + cosf(rotation) * bodyWidth * 0.4f - sinf(rotation) * bodyLength * 0.4f
		};
		DrawTriangle(dorsal1, dorsal2, dorsal3, fish.finColor); // 绘制背鳍
	}
	
	// 计算眼睛位置
	Vector2 eyePos = (Vector2){
		center.x + cosf(rotation) * bodyLength * 0.1f + sinf(rotation) * bodyWidth * 0.1f,
		center.y + sinf(rotation) * bodyLength * 0.1f - cosf(rotation) * bodyWidth * 0.1f
	};
	// 绘制眼睛（黑色+白色高光）
	DrawCircle((int)eyePos.x, (int)eyePos.y, fish.size * 0.08f, fish.eyeColor);
	DrawCircle((int)(eyePos.x + fish.size * 0.02f), (int)(eyePos.y - fish.size * 0.02f), 
			   fish.size * 0.03f, WHITE);
	
	// 玩家无敌状态绘制闪烁光环
	if (fish.type == 0 && isInvincible) {
		float alpha = (sinf(gameTime * 10.0f) + 1.0f) * 0.5f; // 闪烁透明度
		unsigned char alphaChar = (unsigned char)(alpha * 200.0f);
		DrawCircleLines((int)center.x, (int)center.y, fish.size + 8.0f, 
						(Color){255, 255, 0, alphaChar}); // 黄色光环
	}
}

// 绘制主菜单
void DrawMainMenu(void) {
	// 绘制蓝色渐变背景
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		float depth = (float)y / SCREEN_HEIGHT;
		int blue = 50 + (int)(depth * 150);
		Color bgColor = (Color){0, 0, blue, 255};
		DrawLine(0, y, SCREEN_WIDTH, y, bgColor);
	}
	
	// 绘制游戏标题
	DrawText("DEEP SEA ADVENTURE", SCREEN_WIDTH/2 - 300, 100, 60, (Color){0, 200, 255, 255});
	DrawText("Big Fish Eat Small Fish", SCREEN_WIDTH/2 - 200, 180, 30, (Color){100, 200, 255, 255});
	
	// 绘制最高分显示框
	DrawRectangle(SCREEN_WIDTH/2 - 300, 250, 600, 100, (Color){0, 0, 0, 150});
	DrawRectangleLines(SCREEN_WIDTH/2 - 300, 250, 600, 100, (Color){0, 150, 255, 255});
	
	DrawText("HIGH SCORES", SCREEN_WIDTH/2 - 80, 265, 30, (Color){255, 215, 0, 255});
	DrawText(TextFormat("ENDLESS MODE: %d", highScoreEndless), SCREEN_WIDTH/2 - 120, 300, 25, (Color){0, 200, 255, 255});
	DrawText(TextFormat("LEVEL MODE: %d", highScoreLevels), SCREEN_WIDTH/2 - 120, 330, 25, (Color){255, 100, 100, 255});
	
	// 绘制模式选择框
	DrawRectangle(SCREEN_WIDTH/2 - 250, 380, 500, 350, (Color){0, 0, 0, 150});
	DrawRectangleLines(SCREEN_WIDTH/2 - 250, 380, 500, 350, (Color){0, 150, 255, 255});
	
	DrawText("SELECT GAME MODE", SCREEN_WIDTH/2 - 120, 400, 25, WHITE);
	
	// 绘制无尽模式按钮
	Color endlessColor = (Color){0, 100, 200, 255};
	Rectangle endlessRect = {SCREEN_WIDTH/2 - 200, 450, 400, 80};
	
	if (CheckCollisionPointRec(GetMousePosition(), endlessRect)) {
		endlessColor = (Color){0, 150, 255, 255}; // 鼠标悬停变色
	}
	
	DrawRectangle(endlessRect.x, endlessRect.y, endlessRect.width, endlessRect.height, endlessColor);
	DrawRectangleLines(endlessRect.x, endlessRect.y, endlessRect.width, endlessRect.height, WHITE);
	DrawText("ENDLESS MODE", SCREEN_WIDTH/2 - 100, 480, 25, WHITE);
	DrawText("No limit, survive as long as you can", SCREEN_WIDTH/2 - 180, 510, 20, (Color){200, 220, 255, 255});
	DrawText(TextFormat("HIGH SCORE: %d", highScoreEndless), SCREEN_WIDTH/2 - 100, 540, 20, (Color){255, 215, 0, 255});
	
	// 绘制关卡模式按钮
	Color levelColor = (Color){200, 50, 50, 255};
	Rectangle levelRect = {SCREEN_WIDTH/2 - 200, 570, 400, 80};
	
	if (CheckCollisionPointRec(GetMousePosition(), levelRect)) {
		levelColor = (Color){255, 100, 100, 255}; // 鼠标悬停变色
	}
	
	DrawRectangle(levelRect.x, levelRect.y, levelRect.width, levelRect.height, levelColor);
	DrawRectangleLines(levelRect.x, levelRect.y, levelRect.width, levelRect.height, WHITE);
	DrawText("LEVEL MODE", SCREEN_WIDTH/2 - 80, 600, 25, WHITE);
	DrawText("Complete 3 levels with BOSS fights", SCREEN_WIDTH/2 - 170, 630, 20, (Color){255, 220, 200, 255});
	DrawText(TextFormat("HIGH SCORE: %d", highScoreLevels), SCREEN_WIDTH/2 - 100, 660, 20, (Color){255, 215, 0, 255});
	
	// 绘制退出按钮
	Color quitColor = (Color){100, 100, 100, 255};
	Rectangle quitRect = {SCREEN_WIDTH/2 - 200, 670, 400, 60};
	
	if (CheckCollisionPointRec(GetMousePosition(), quitRect)) {
		quitColor = (Color){150, 150, 150, 255}; // 鼠标悬停变色
	}
	
	DrawRectangle(quitRect.x, quitRect.y, quitRect.width, quitRect.height, quitColor);
	DrawRectangleLines(quitRect.x, quitRect.y, quitRect.width, quitRect.height, WHITE);
	DrawText("QUIT GAME", SCREEN_WIDTH/2 - 60, 690, 25, WHITE);
	
	// 绘制控制说明
	DrawText("CONTROLS: WASD or Mouse to move, eat smaller fish, avoid bigger ones", 
			 SCREEN_WIDTH/2 - 300, 750, 20, (Color){200, 200, 200, 255});
}

// 绘制无缝背景（海洋效果）
void DrawSeamlessBackground(void) {
	// 绘制蓝色渐变背景
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		float depth = (float)y / SCREEN_HEIGHT;
		
		int blue = 100 + (int)(depth * 100);
		int green = 20;
		
		// 颜色范围限制
		if (blue < 100) blue = 100;
		if (blue > 255) blue = 255;
		if (green < 20) green = 20;
		if (green > 40) green = 40;
		
		Color waterColor = (Color){0, green, blue, 255};
		DrawLine(0, y, SCREEN_WIDTH, y, waterColor);
	}
	
	// 绘制水泡效果
	for (int i = 0; i < 20; i++) {
		float x = sinf(gameTime + i * 0.5f) * 60.0f + i * 50.0f;
		float y = fmodf(gameTime * 40.0f + i * 25.0f, SCREEN_HEIGHT);
		float size = 1.0f + sinf(gameTime + i) * 0.5f;
		
		DrawCircle(x, y, size, (Color){255, 255, 255, 150}); // 白色半透明水泡
	}
}

// 绘制UI界面
void DrawUI(void) {
	if (currentMode == MODE_ENDLESS) {
		// 无尽模式UI
		DrawRectangle(10, 10, 400, 200, (Color){0, 0, 0, 180}); // 半透明背景
		DrawRectangleLines(10, 10, 400, 200, (Color){0, 150, 255, 255}); // 边框
		
		DrawText("ENDLESS MODE", 20, 20, 24, (Color){0, 200, 255, 255}); // 模式名称
		DrawText(TextFormat("SCORE: %d", score), 20, 60, 22, YELLOW); // 当前分数
		DrawText(TextFormat("HIGH SCORE: %d", highScoreEndless), 20, 90, 20, (Color){255, 215, 0, 255}); // 最高分
		DrawText(TextFormat("SIZE: %.1f", player.size), 20, 120, 20, GREEN); // 玩家大小
		DrawText(TextFormat("LIVES: %d/%d", lives, MAX_LIVES), 20, 150, 20, RED); // 生命值
		
	} else {
		// 关卡模式UI
		DrawRectangle(10, 10, 400, 200, (Color){0, 0, 0, 180}); // 半透明背景
		DrawRectangleLines(10, 10, 400, 200, (Color){200, 50, 50, 255}); // 边框
		
		DrawText("LEVEL MODE", 20, 20, 24, (Color){255, 100, 100, 255}); // 模式名称
		DrawText(TextFormat("SCORE: %d", score), 20, 60, 22, YELLOW); // 当前分数
		DrawText(TextFormat("HIGH SCORE: %d", highScoreLevels), 20, 90, 20, (Color){255, 215, 0, 255}); // 最高分
		DrawText(TextFormat("LEVEL: %d/%d", level, maxLevel), 20, 120, 20, (Color){255, 150, 50, 255}); // 当前关卡
		DrawText(TextFormat("SIZE: %.1f/%d", player.size, (int)WIN_SIZE), 20, 150, 20, GREEN); // 玩家大小/胜利大小
	}
	
	// 右侧统计信息UI
	DrawRectangle(SCREEN_WIDTH - 310, 10, 300, 200, (Color){0, 0, 0, 180}); // 半透明背景
	DrawRectangleLines(SCREEN_WIDTH - 310, 10, 300, 200, WHITE); // 边框
	
	DrawText(TextFormat("SMALL FISH: %d", activeSmallFishCount), SCREEN_WIDTH - 300, 20, 20, (Color){200, 200, 100, 255}); // 小鱼数量
	DrawText(TextFormat("BIG FISH: %d", activeEnemyFishCount), SCREEN_WIDTH - 300, 50, 20, (Color){255, 100, 100, 255}); // 敌人鱼数量
	DrawText(TextFormat("ENEMY SPEED: %.1f", enemyBaseSpeed), SCREEN_WIDTH - 300, 80, 20, (Color){255, 150, 150, 255}); // 敌人速度
	DrawText(TextFormat("FISH EATEN: %d", fishEaten), SCREEN_WIDTH - 300, 110, 20, WHITE); // 吃掉的鱼数量
	DrawText(TextFormat("TIME: %.1fs", gameTime), SCREEN_WIDTH - 300, 140, 20, WHITE); // 游戏时间
	
	// 进度条（玩家大小/胜利大小）
	float progress = player.size / WIN_SIZE;
	if (progress > 1.0f) progress = 1.0f; // 限制最大进度
	
	DrawRectangle(SCREEN_WIDTH/2 - 200, 20, 400, 25, (Color){0, 0, 0, 150}); // 进度条背景
	
	// 根据模式选择进度条颜色
	Color progressColor = (currentMode == MODE_ENDLESS) ? 
	(Color){0, 200, 255, 255} : (Color){255, 100, 100, 255};
	
	DrawRectangle(SCREEN_WIDTH/2 - 200, 20, (int)(progress * 400), 25, progressColor); // 进度条
	DrawRectangleLines(SCREEN_WIDTH/2 - 200, 20, 400, 25, WHITE); // 进度条边框
	
	// 进度条文字
	if (currentMode == MODE_ENDLESS) {
		DrawText(TextFormat("SIZE: %.1f", player.size), SCREEN_WIDTH/2 - 50, 22, 20, BLACK);
	} else {
		DrawText(TextFormat("PROGRESS: %.1f%%", progress * 100), SCREEN_WIDTH/2 - 80, 22, 20, BLACK);
	}
	
	// 底部控制说明
	DrawRectangle(10, SCREEN_HEIGHT - 50, 600, 40, (Color){0, 0, 0, 180}); // 半透明背景
	DrawText("CONTROLS: WASD/MOUSE | R: RESTART | ESC: MENU | M: MUTE", 20, SCREEN_HEIGHT - 40, 20, WHITE); // 控制说明
	
	// 无尽模式显示下一波鱼生成倒计时
	if (currentMode == MODE_ENDLESS) {
		float timeLeft = FISH_SPAWN_INTERVAL - fishSpawnTimer;
		if (timeLeft < 3.0f) {
			DrawRectangle(SCREEN_WIDTH/2 - 150, 60, 300, 30, (Color){0, 0, 0, 150}); // 倒计时背景
			DrawText(TextFormat("NEW FISH IN: %.1fs", timeLeft), 
					 SCREEN_WIDTH/2 - 100, 65, 20, (Color){100, 255, 100, 255}); // 倒计时文字
		}
	}
}

// ===================== 游戏逻辑更新函数 =====================
// 更新玩家状态（输入处理、移动、边界、相机）
void UpdatePlayer(void) {
	if (gameState != GAME_PLAYING) return; // 非游戏中不执行
	
	Vector2 input = {0.0f, 0.0f}; // 输入向量
	
	// 键盘输入处理（WASD/方向键）
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y = -1.0f;
	if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y = 1.0f;
	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x = -1.0f;
	if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x = 1.0f;
	
	// 鼠标输入处理（左键按住）
	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		Vector2 mousePos = GetMousePosition();
		float screenCenterX = SCREEN_WIDTH / 2.0f;
		float screenCenterY = SCREEN_HEIGHT / 2.0f;
		
		// 转换鼠标坐标到世界坐标
		float worldX = cameraX + (mousePos.x - screenCenterX) / cameraZoom;
		float worldY = cameraY + (mousePos.y - screenCenterY) / cameraZoom;
		
		// 计算朝向鼠标的输入向量
		input.x = worldX - player.position.x;
		input.y = worldY - player.position.y;
	}
	
	// 归一化输入向量（保持移动速度一致）
	float length = sqrtf(input.x * input.x + input.y * input.y);
	if (length > 0.0f) {
		input.x /= length;
		input.y /= length;
	}
	
	// 优化速度计算：玩家变大后速度应该减小，但要保持足够的追鱼速度
	float speed = 6.0f;  // 提高基础速度到6.0
	if (player.size > INITIAL_PLAYER_SIZE) {
		// 新代码：分段速度控制
		if (player.size <= 40.0f) {  // 小尺寸阶段
			speed = 5.8f - (player.size - INITIAL_PLAYER_SIZE) * 0.005f;
		} else if (player.size <= 80.0f) {  // 中尺寸阶段
			speed = 4.5f;
		} else {  // 大尺寸阶段
			speed = 4.2f;  // 保持较高速度
		}
		
		if (speed < 4.0f) speed = 4.0f;  // 提高最小速度限制
	}
	
	// 冲刺机制：按住Shift键时获得额外速度
	if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
		speed *= 1.5f;  // 冲刺时速度提高50%
	}
	
	// 更新玩家速度向量
	player.velocity.x = input.x * speed;
	player.velocity.y = input.y * speed;
	
	// 更新玩家位置
	player.position.x += player.velocity.x;
	player.position.y += player.velocity.y;
	
	// 边界循环（超出屏幕边缘后从另一侧出现）
	if (player.position.x < 0) player.position.x = WORLD_WIDTH;
	if (player.position.x > WORLD_WIDTH) player.position.x = 0;
	if (player.position.y < 0) player.position.y = WORLD_HEIGHT;
	if (player.position.y > WORLD_HEIGHT) player.position.y = 0;
	
	// 更新玩家旋转角度（朝向移动方向）
	if (length > 0.1f) {
		player.rotation = atan2f(player.velocity.y, player.velocity.x);
	}
	
	// 更新相机位置（跟随玩家）
	cameraX = player.position.x;
	cameraY = player.position.y;
	
	// 更新相机缩放（玩家越大视角越远）
	cameraZoom = 1.0f;
	if (player.size > 30.0f) {
		// 减慢缩放速度，保持更多可见区域
		cameraZoom = 1.0f - (player.size - 30.0f) * 0.008f;
		if (cameraZoom < 0.5f) cameraZoom = 0.5f;  // 提高最小缩放限制
	}
	
	// 更新无敌状态计时器
	if (isInvincible) {
		invincibleTimer += GetFrameTime();
		if (invincibleTimer >= INVINCIBLE_TIME) {
			isInvincible = 0; // 结束无敌状态
		}
	}
}

// 检查碰撞（吞噬/被吞噬）
void CheckCollisions(void) {
	if (gameState != GAME_PLAYING) return; // 非游戏中不执行
	
	// 检查与小鱼的碰撞
	for (int i = 0; i < MAX_SMALL_FISH; i++) {
		if (!smallFish[i].active) continue; // 跳过未激活的鱼
		
		// 计算玩家与小鱼的距离
		float distance = FishDistance(player.position.x, player.position.y, 
									  smallFish[i].position.x, smallFish[i].position.y);
		float minDistance = player.size + smallFish[i].size; // 最小碰撞距离
		
		// 碰撞判定
		if (distance < minDistance * 0.9f) {
			// 玩家足够大，可以吞噬小鱼
			if (player.size > smallFish[i].size * EAT_THRESHOLD) {
				smallFish[i].active = 0; // 小鱼消失
				score += smallFish[i].value; // 加分
				player.size += 1.5f; // 玩家变大
				fishEaten++; // 吃掉的鱼数量+1
				activeSmallFishCount--; // 活跃小鱼数-1
				
				// 关卡模式检查胜利条件
				if (currentMode == MODE_LEVELS && player.size >= WIN_SIZE) {
					level++; // 进入下一关
					if (level > maxLevel) {
						CheckAchievements(); // 检查成就
						gameState = GAME_CERTIFICATE; // 显示胜利证书
						certificateShowTime = 5.0f;
					} else {
						InitGame(MODE_LEVELS); // 重新初始化当前关卡
					}
				}
			}
		}
	}
	
	// 检查与敌人鱼的碰撞
	for (int i = 0; i < MAX_ENEMY_FISH; i++) {
		if (!enemyFish[i].active) continue; // 跳过未激活的鱼
		
		// 计算玩家与敌人鱼的距离
		float distance = FishDistance(player.position.x, player.position.y, 
									  enemyFish[i].position.x, enemyFish[i].position.y);
		float minDistance = player.size + enemyFish[i].size; // 最小碰撞距离
		
		// 碰撞判定
		if (distance < minDistance * 0.9f) {
			// 敌人鱼足够大，玩家非无敌状态
			if (enemyFish[i].size > player.size * EAT_THRESHOLD && !isInvincible) {
				lives--; // 生命值-1
				isInvincible = 1; // 进入无敌状态
				invincibleTimer = 0.0f; // 重置无敌计时器
				
				if (lives <= 0) {
					UpdateHighScore(); // 更新最高分
					gameState = GAME_OVER; // 游戏结束
				}
			} 
			// 玩家足够大，可以吞噬敌人鱼
			else if (player.size > enemyFish[i].size * EAT_THRESHOLD) {
				enemyFish[i].active = 0; // 敌人鱼消失
				score += enemyFish[i].value; // 加分
				player.size += 3.0f; // 玩家变大
				fishEaten++; // 吃掉的鱼数量+1
				activeEnemyFishCount--; // 活跃敌人鱼数-1
				
				// 关卡模式检查胜利条件
				if (currentMode == MODE_LEVELS && player.size >= WIN_SIZE) {
					if (level < maxLevel) {  // 如果还有下一关
						level++;
						InitGame(MODE_LEVELS);
					} else {  // 完成所有关卡
						CheckAchievements();
						gameState = GAME_CERTIFICATE;
						certificateShowTime = 5.0f;
					}
				}
			}
		}
	}
}

// 重生鱼（无尽模式）
void RespawnFish(void) {
	if (currentMode == MODE_ENDLESS) {
		respawnTimer++; // 累加重生计时器
		
		// 每60帧检查一次
		if (respawnTimer >= 60) {
			respawnTimer = 0;
			
			// 小鱼数量不足时重生
			if (activeSmallFishCount < 30) {
				SpawnSmallFish();
			}
			// 敌人鱼数量不足时重生
			if (activeEnemyFishCount < 6) {
				SpawnEnemyFish();
			}
		}
	}
}

// ===================== 主函数 =====================
int main(void) {
	// 初始化窗口
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Deep Sea Adventure");
	SetTargetFPS(60); // 设置帧率
	
	// 创建渲染纹理（解决闪烁问题）
	gameTexture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	LoadHighScores(); // 加载最高分
	
	gameState = GAME_MENU; // 初始状态：主菜单
	
	// 游戏主循环
	while (!WindowShouldClose()) {
		float deltaTime = GetFrameTime(); // 获取帧时间
		gameTime += deltaTime; // 累加游戏时间
		
		// 主菜单状态处理
		if (gameState == GAME_MENU) {
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				Vector2 mousePos = GetMousePosition(); // 获取鼠标位置
				
				// 点击无尽模式按钮
				if (CheckCollisionPointRec(mousePos, (Rectangle){SCREEN_WIDTH/2 - 200, 450, 400, 80})) {
					InitGame(MODE_ENDLESS); // 初始化无尽模式
					gameState = GAME_PLAYING; // 进入游戏
				}
				// 点击关卡模式按钮
				else if (CheckCollisionPointRec(mousePos, (Rectangle){SCREEN_WIDTH/2 - 200, 570, 400, 80})) {
					InitGame(MODE_LEVELS); // 初始化关卡模式
					gameState = GAME_PLAYING; // 进入游戏
				}
				// 点击退出按钮
				else if (CheckCollisionPointRec(mousePos, (Rectangle){SCREEN_WIDTH/2 - 200, 670, 400, 60})) {
					break; // 退出循环
				}
			}
		} 
		// 游戏进行中状态处理
		else if (gameState == GAME_PLAYING) {
			// 按ESC返回菜单
			if (IsKeyPressed(KEY_ESCAPE)) {
				UpdateHighScore(); // 更新最高分
				gameState = GAME_MENU; // 返回菜单
				continue;
			}
			
			// 按R重新开始
			if (IsKeyPressed(KEY_R)) {
				InitGame(currentMode); // 重新初始化游戏
			}
			
			// 按M静音
			if (IsKeyPressed(KEY_M)) {
				isMuted = !isMuted; // 切换静音状态
			}
			
			UpdatePlayer(); // 更新玩家状态
			
			// 更新小鱼AI和位置
			for (int i = 0; i < MAX_SMALL_FISH; i++) {
				if (smallFish[i].active) {
					// 计算玩家与小鱼的距离和方向
					float dx = player.position.x - smallFish[i].position.x;
					float dy = player.position.y - smallFish[i].position.y;
					float distance = FishDistance(smallFish[i].position.x, smallFish[i].position.y, 
												  player.position.x, player.position.y);
					
					// 玩家足够大且距离近，小鱼逃跑
					if (distance < SMALL_FISH_FLEE_DISTANCE && player.size > smallFish[i].size * EAT_THRESHOLD) {
						smallFish[i].velocity.x = -dx / distance * 2.0f;
						smallFish[i].velocity.y = -dy / distance * 2.0f;
					}
					
					// 更新小鱼位置
					smallFish[i].position.x += smallFish[i].velocity.x;
					smallFish[i].position.y += smallFish[i].velocity.y;
					
					// 边界循环
					if (smallFish[i].position.x < 0) smallFish[i].position.x = WORLD_WIDTH;
					if (smallFish[i].position.x > WORLD_WIDTH) smallFish[i].position.x = 0;
					if (smallFish[i].position.y < 0) smallFish[i].position.y = WORLD_HEIGHT;
					if (smallFish[i].position.y > WORLD_HEIGHT) smallFish[i].position.y = 0;
					
					// 更新小鱼旋转角度
					if (smallFish[i].velocity.x != 0.0f || smallFish[i].velocity.y != 0.0f) {
						smallFish[i].rotation = atan2f(smallFish[i].velocity.y, smallFish[i].velocity.x);
					}
				}
			}
			
			// 更新敌人鱼AI和位置
			for (int i = 0; i < MAX_ENEMY_FISH; i++) {
				if (enemyFish[i].active) {
					// 计算玩家与敌人鱼的距离和方向
					float dx = player.position.x - enemyFish[i].position.x;
					float dy = player.position.y - enemyFish[i].position.y;
					float distance = FishDistance(enemyFish[i].position.x, enemyFish[i].position.y, 
												  player.position.x, player.position.y);
					
					// 距离足够近时触发AI行为
					if (distance < ENEMY_CHASE_DISTANCE) {
						// 敌人鱼更大，追击玩家
						if (enemyFish[i].size > player.size * EAT_THRESHOLD) {
							enemyFish[i].velocity.x += dx / distance * 0.05f;
							enemyFish[i].velocity.y += dy / distance * 0.05f;
						}
						// 玩家更大，敌人鱼逃跑
						else if (player.size > enemyFish[i].size * EAT_THRESHOLD) {
							enemyFish[i].velocity.x -= dx / distance * 0.1f;
							enemyFish[i].velocity.y -= dy / distance * 0.1f;
						}
					}
					
					// 限制敌人鱼速度
					float speed = sqrtf(enemyFish[i].velocity.x*enemyFish[i].velocity.x + enemyFish[i].velocity.y*enemyFish[i].velocity.y);
					if (speed > enemyFish[i].currentSpeed) {
						enemyFish[i].velocity.x = enemyFish[i].velocity.x / speed * enemyFish[i].currentSpeed;
						enemyFish[i].velocity.y = enemyFish[i].velocity.y / speed * enemyFish[i].currentSpeed;
					}
					
					// 更新敌人鱼位置
					enemyFish[i].position.x += enemyFish[i].velocity.x;
					enemyFish[i].position.y += enemyFish[i].velocity.y;
					
					// 边界循环
					if (enemyFish[i].position.x < 0) enemyFish[i].position.x = WORLD_WIDTH;
					if (enemyFish[i].position.x > WORLD_WIDTH) enemyFish[i].position.x = 0;
					if (enemyFish[i].position.y < 0) enemyFish[i].position.y = WORLD_HEIGHT;
					if (enemyFish[i].position.y > WORLD_HEIGHT) enemyFish[i].position.y = 0;
					
					// 更新敌人鱼旋转角度
					if (speed > 0.1f) {
						enemyFish[i].rotation = atan2f(enemyFish[i].velocity.y, enemyFish[i].velocity.x);
					}
				}
			}
			
			CheckCollisions(); // 检查碰撞
			
			// 无尽模式重生鱼和生成额外鱼
			if (currentMode == MODE_ENDLESS) {
				RespawnFish();
				SpawnAdditionalFish();
			}
			
		} 
		// 游戏结束状态处理
		else if (gameState == GAME_OVER) {
			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
				Vector2 mousePos = GetMousePosition(); // 获取鼠标位置
				
				// 点击重新开始按钮
				if (CheckCollisionPointRec(mousePos, (Rectangle){SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 200, 50})) {
					InitGame(currentMode); // 重新初始化游戏
					gameState = GAME_PLAYING; // 进入游戏
				} 
				// 点击返回菜单按钮
				else if (CheckCollisionPointRec(mousePos, (Rectangle){SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 190, 200, 50})) {
					UpdateHighScore(); // 更新最高分
					gameState = GAME_MENU; // 返回菜单
				}
			}
			
			// 按R重新开始
			if (IsKeyPressed(KEY_R)) {
				InitGame(currentMode); // 重新初始化游戏
				gameState = GAME_PLAYING; // 进入游戏
			}
		}
		
		// ===================== 渲染部分 =====================
		BeginDrawing(); // 开始绘制
		
		// 主菜单渲染
		if (gameState == GAME_MENU) {
			ClearBackground((Color){0, 0, 50, 255}); // 清空背景
			
			DrawMainMenu(); // 绘制主菜单
		} 
		// 游戏进行中渲染
		else if (gameState == GAME_PLAYING) {
			// 开始渲染到纹理（解决闪烁）
			BeginTextureMode(gameTexture);
			
			ClearBackground((Color){0, 10, 100, 255}); // 清空背景
			
			// 开始2D相机模式
			BeginMode2D((Camera2D){
				.offset = (Vector2){SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f},
				.target = (Vector2){cameraX, cameraY},
				.rotation = 0.0f,
				.zoom = cameraZoom
			});
			
			DrawSeamlessBackground(); // 绘制背景
			
			// 绘制所有小鱼
			for (int i = 0; i < MAX_SMALL_FISH; i++) {
				if (smallFish[i].active) DrawFish(smallFish[i]);
			}
			
			// 绘制所有敌人鱼
			for (int i = 0; i < MAX_ENEMY_FISH; i++) {
				if (enemyFish[i].active) DrawFish(enemyFish[i]);
			}
			
			DrawFish(player); // 绘制玩家
			
			EndMode2D(); // 结束2D相机模式
			
			DrawUI(); // 绘制UI
			
			EndTextureMode(); // 结束纹理渲染
			
			// 绘制纹理到屏幕
			DrawTextureRec(gameTexture.texture, 
						   (Rectangle){0, 0, (float)gameTexture.texture.width, (float)-gameTexture.texture.height}, 
						   (Vector2){0, 0}, WHITE);
			
		} 
		// 游戏结束渲染
		else if (gameState == GAME_OVER) {
			ClearBackground((Color){0, 0, 50, 255}); // 清空背景
			
			// 开始2D相机模式
			BeginMode2D((Camera2D){
				.offset = (Vector2){SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f},
				.target = (Vector2){cameraX, cameraY},
				.rotation = 0.0f,
				.zoom = cameraZoom
			});
			
			DrawSeamlessBackground(); // 绘制背景
			
			// 绘制所有小鱼
			for (int i = 0; i < MAX_SMALL_FISH; i++) {
				if (smallFish[i].active) DrawFish(smallFish[i]);
			}
			
			// 绘制所有敌人鱼
			for (int i = 0; i < MAX_ENEMY_FISH; i++) {
				if (enemyFish[i].active) DrawFish(enemyFish[i]);
			}
			
			DrawFish(player); // 绘制玩家
			
			EndMode2D(); // 结束2D相机模式
			
			DrawUI(); // 绘制UI
			
			// 绘制游戏结束遮罩
			DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
			
			// 绘制游戏结束文字
			DrawText("GAME OVER", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - 50, 50, RED);
			DrawText(TextFormat("SCORE: %d", score), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, 30, YELLOW);
			
			// 绘制统计信息
			DrawText(TextFormat("FISH EATEN: %d", fishEaten), SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 50, 30, WHITE);
			DrawText(TextFormat("TIME SURVIVED: %.1fs", gameTime), SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 100, 30, SKYBLUE);
			
			// 绘制重新开始按钮
			Color restartColor = (Color){0, 150, 50, 255};
			if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 200, 50})) {
				restartColor = (Color){0, 200, 80, 255}; // 鼠标悬停变色
			}
			DrawRectangle(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 200, 50, restartColor);
			DrawRectangleLines(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 200, 50, WHITE);
			DrawText("PLAY AGAIN", SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 + 135, 20, BLACK);
			
			// 绘制返回菜单按钮
			Color menuColor = (Color){100, 100, 100, 255};
			if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 190, 200, 50})) {
				menuColor = (Color){150, 150, 150, 255}; // 鼠标悬停变色
			}
			DrawRectangle(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 190, 200, 50, menuColor);
			DrawRectangleLines(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 190, 200, 50, WHITE);
			// 绘制重新开始按钮背景（矩形）
			DrawRectangle(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 200, 50, restartColor);
			// 绘制重新开始按钮的白色边框
			DrawRectangleLines(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 120, 200, 50, WHITE);
			// 绘制重新开始按钮文字（PLAY AGAIN），居中对齐
			DrawText("PLAY AGAIN", SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 + 135, 20, BLACK);
			
			// 初始化返回主菜单按钮的基础颜色（深灰色）
			menuColor = (Color){100, 100, 100, 255};
			// 检测鼠标是否悬停在返回主菜单按钮的矩形区域内
			if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 190, 200, 50})) {
				// 鼠标悬停时按钮颜色变浅（浅灰色），提升交互反馈
				menuColor = (Color){150, 150, 150, 255};
			}
			// 绘制返回主菜单按钮背景（矩形）
			DrawRectangle(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 190, 200, 50, menuColor);
			// 绘制返回主菜单按钮的白色边框
			DrawRectangleLines(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 190, 200, 50, WHITE);
			// 绘制返回主菜单按钮文字（MAIN MENU），居中对齐
			DrawText("MAIN MENU", SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 + 205, 20, BLACK);
		}
		
		// 结束当前帧的所有绘制操作（Raylib核心函数）
		EndDrawing();
	}
	
	// 游戏退出前更新最高分（对比当前分数与历史最高分）
	UpdateHighScore();
	// 将最新的最高分保存到
	// 释放渲染纹理资源（避免内存泄漏）
	UnloadRenderTexture(gameTexture);
// 关闭游戏窗口，释放窗口资源
	CloseWindow();
// 程序正常退出，返回0表示无错误
	return 0;
}

