/*
 *        Computer Graphics Course - Shenzhen University
 *    Mid-term Assignment - Tetris implementation sample code
 * ============================================================
 *
 * - 本代码仅仅是参考代码，具体要求请参考作业说明，按照顺序逐步完成。
 * - 关于配置OpenGL开发环境、编译运行，请参考第一周实验课程相关文档。
 *
 * - 已实现功能如下：
 * - 1) 绘制棋盘格和‘L’型方块
 * - 2) 键盘左/右/下键控制方块的移动，上键旋转方块
 *
 * - 未实现功能如下：
 * - 1) 绘制‘J’、‘Z’等形状的方块
 * - 2) 随机生成方块并赋上不同的颜色
 * - 3) 方块的自动向下移动
 * - 4) 方块之间、方块与边界之间的碰撞检测
 * - 5) 棋盘格中每一行填充满之后自动消除
 * - 6) 其他
 *
 * - 自己实现的功能如下：
 * - 1) 绘制‘J’、‘Z’等七种形状的方块（allRotationsLshape[7][4][4]）
 * - 2) 随机生成方块并赋上了七种不同的颜色(colors[7])
 * - 3) 方块的自动向下移动（falling函数实现）
 * - 4) 方块之间、方块与边界之间的碰撞检测（checkvaild函数实现）
 * - 5) 棋盘格中每一行填充满之后自动消除，并计算积分，增加游戏乐趣（checkfullrow函数实现）
 * - 6) 实现了游戏的暂停与恢复，方便玩家
 * - 7) 方块向上移动，作为辅助技能
 * - 8) 每达到一定积分就加快方块下落速度，挑战升级
*/

#include "Angel.h"

#include <cstdlib>
#include <iostream>
#include <string>

// 自己添加的头文件
#include <GL\freeglut.h>
using namespace std;

int starttime;			// 控制方块向下移动时间
int rotation = 0;		// 控制当前窗口中的方块旋转
glm::vec2 tile[4];			// 表示当前窗口中的方块
bool gameover = false;	// 游戏结束控制变量
int xsize = 400;		// 窗口大小（尽量不要变动窗口大小！）
int ysize = 720;

// 自己添加或修改的新变量
int icolor; //方块颜色
int ishape; //方块形状
int fall_interval = 1000; //方块自动下落的时间间隔
int current_time; //当前时间
int num = 1; //下落时间间隔个数
int point = 0; //每消除一行所得积分
bool gamestop = false; //用来表示游戏是否暂停的变量
int hard_level = 1; //游戏难度
bool BUG = false; //用于开启bug模式
glm::vec4 board_color[10][20]; //每个格子的颜色


// 单个网格大小
int tile_width = 33;

// 网格布大小
const int board_width = 10;
const int board_height = 20;


// 网格三角面片的顶点数量
const int points_num = board_height * board_width * 6;

// 我们用画直线的方法绘制网格
// 包含竖线 board_width+1 条
// 包含横线 board_height+1 条
// 一条线2个顶点坐标
// 网格线的数量
const int board_line_num =  (board_width + 1) + (board_height + 1);


// 一个二维数组表示所有可能出现的方块和方向。一共七种类型的方块
glm::vec2 allRotationsLshape[7][4][4] =
{
	//以下按照实验指导的图形顺序，根据逆时针旋转90度
	//1：O型方块
	{{glm::vec2(0, 0), glm::vec2(-1,0),glm::vec2(0, -1), glm::vec2(-1,-1)},
    {glm::vec2(0, 0),glm::vec2(-1,0),glm::vec2(0, -1), glm::vec2(-1,-1)},
    {glm::vec2(0, 0), glm::vec2(-1,0), glm::vec2(0, -1),glm::vec2(-1,-1)},
    {glm::vec2(0, 0), glm::vec2(-1,0), glm::vec2(0, -1),glm::vec2(-1,-1)}},

   //2:I型方块
	{{glm::vec2(-2, 0), glm::vec2(-1,0), glm::vec2(1, 0), glm::vec2(0,0)},
	{glm::vec2(0, 1),glm::vec2(0, 0), glm::vec2(0,-1), glm::vec2(0, -2)},
	{glm::vec2(-2, 0), glm::vec2(-1,0), glm::vec2(1, 0), glm::vec2(0,0)},
	{glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0,-1), glm::vec2(0, -2)}},

	//3:S型方块
	{{glm::vec2(0, 0),glm::vec2(0,-1),glm::vec2(-1, -1), glm::vec2(1,0)},
	{glm::vec2(0, 1),glm::vec2(0, 0), glm::vec2(1,0), glm::vec2(1, -1)},
	{glm::vec2(0, 0), glm::vec2(0,-1),glm::vec2(-1, -1),glm::vec2(1,0)},
	{glm::vec2(0, 1), glm::vec2(0, 0),glm::vec2(1,0),glm::vec2(1, -1)}},

	//4:Z型方块
	{{glm::vec2(-1, 0), glm::vec2(0,0), glm::vec2(0, -1), glm::vec2(1,-1)},
	{glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(1,0),glm::vec2(1, 1)},
	{glm::vec2(-1, 0),glm::vec2(0,0), glm::vec2(0, -1),glm::vec2(1,-1)},
	{glm::vec2(0, 0), glm::vec2(0, -1), glm::vec2(1,0), glm::vec2(1, 1)}},

	//5:L型方块
    {{glm::vec2(0, 0),glm::vec2(-1,0), glm::vec2(1, 0),glm::vec2(-1,-1)},
	{glm::vec2(0, 1),glm::vec2(0, 0),glm::vec2(0,-1), glm::vec2(1, -1)},
	{glm::vec2(1, 1), glm::vec2(-1,0), glm::vec2(0, 0),glm::vec2(1,  0)},
	{glm::vec2(-1,1), glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(0, -1)}},

	//6:J型方块
	{{glm::vec2(-1, 0), glm::vec2(0,0), glm::vec2(1, 0), glm::vec2(1,-1)},
	{glm::vec2(0, 1),glm::vec2(0, 0),glm::vec2(0,-1), glm::vec2(1, 1)},
	{glm::vec2(-1, 0),glm::vec2(0,0), glm::vec2(1, 0),glm::vec2(-1,  1)},
	{glm::vec2(-1,-1), glm::vec2(0, -1),glm::vec2(0, 0), glm::vec2(0, 1)}},

	//7:T型方块
	{{glm::vec2(-1, 0),glm::vec2(0,0), glm::vec2(1, 0),glm::vec2(0,-1)},
	{glm::vec2(0, -1), glm::vec2(0, 0),glm::vec2(0,1), glm::vec2(1, 0)},
	{glm::vec2(-1, 0), glm::vec2(0,0),glm::vec2(1, 0),glm::vec2(0,  1)},
	{glm::vec2(-1,0), glm::vec2(0, -1), glm::vec2(0, 0), glm::vec2(0, 1)}},

};

// 绘制窗口的颜色变量
glm::vec4 orange = glm::vec4(1.0, 0.5, 0.0, 1.0);
glm::vec4 white = glm::vec4(1.0, 1.0, 1.0, 1.0);
glm::vec4 black = glm::vec4(0.0, 0.0, 0.0, 1.0);

// 自己新添加的颜色
glm::vec4 pink = glm::vec4(1.0, 0.75, 0.8, 1.0);
glm::vec4 yellow = glm::vec4(1.0, 1.0, 0.0, 1.0);
glm::vec4 red = glm::vec4(1.0, 0.0, 0.0, 1.0);
glm::vec4 green = glm::vec4(0.0, 1.0, 0.0, 1.0);
glm::vec4 blue = glm::vec4(0.0, 0.0, 1.0, 1.0);
glm::vec4 colors[7] = { orange, white, pink, yellow, red, green, blue};//七种不同的颜色

// 当前方块的位置（以棋盘格的左下角为原点的坐标系）
glm::vec2 tilepos = glm::vec2(5, 19);

// 布尔数组表示棋盘格的某位置是否被方块填充，即board[x][y] = true表示(x,y)处格子被填充。
// （以棋盘格的左下角为原点的坐标系）
bool board[board_width][board_height];

// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色
glm::vec4 board_colours[points_num];

GLuint locxsize;
GLuint locysize;

GLuint vao[3];
GLuint vbo[6];

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 修改棋盘格在pos位置的颜色为colour，并且更新对应的VBO
void changecellcolour(glm::vec2 pos, glm::vec4 colour)
{
	//用二维数组记录对应格子的颜色
	board_color[int(pos.x)][int(pos.y)] = colour;

	// 每个格子是个正方形，包含两个三角形，总共6个定点，并在特定的位置赋上适当的颜色
	for (int i = 0; i < 6; i++)
		board_colours[(int)( 6 * ( board_width*pos.y + pos.x) + i)] = colour;

	glm::vec4 newcolours[6] = {colour, colour, colour, colour, colour, colour};

	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);

	// 计算偏移量，在适当的位置赋上颜色
	int offset = 6 * sizeof(glm::vec4) * (int)( board_width * pos.y + pos.x);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// 当前方块移动或者旋转时，更新VBO
void updatetile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{
		// 计算格子的坐标值
		GLfloat x = tilepos.x + tile[i].x;
		GLfloat y = tilepos.y + tile[i].y;

		glm::vec4 p1 = glm::vec4(tile_width + (x * tile_width), tile_width + (y * tile_width), .4, 1);
		glm::vec4 p2 = glm::vec4(tile_width + (x * tile_width), tile_width*2 + (y * tile_width), .4, 1);
		glm::vec4 p3 = glm::vec4(tile_width*2 + (x * tile_width), tile_width + (y * tile_width), .4, 1);
		glm::vec4 p4 = glm::vec4(tile_width*2 + (x * tile_width), tile_width*2 + (y * tile_width), .4, 1);

		// 每个格子包含两个三角形，所以有6个顶点坐标
		glm::vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4};
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(glm::vec4), 6*sizeof(glm::vec4), newpoints);
	}
	glBindVertexArray(0);
}

// 设置当前方块为下一个即将出现的方块。在游戏开始的时候调用来创建一个初始的方块，
// 在游戏结束的时候判断，没有足够的空间来生成新的方块。
void newtile()
{
	// 判断游戏是否结束
	bool isfull = false; //判断上面二行中间是否被填满，填满则游戏结束
	for (int i = 18; i < 20; i++) { //最上面两行
		for (int j = 4; j < 7; j++) { //中间几格
			if (board[j][i] == true)  
				isfull = true;
		}
	}

	// 游戏结束
	if (isfull == true) {
		cout <<"游戏结束，按下R键重新开始游戏，按下Q或Esc键退出游戏。"<< endl;
		gameover = true;
		return;
	}

	// 游戏未结束
	// 将新方块放于棋盘格的最上行中间位置并设置默认的旋转方向
	tilepos = glm::vec2(5 , 19);
	rotation = 0;

	// 利用随机数获取新方块形状
	srand(time(0)); //改变随机数种子
	ishape = (starttime + rand()) % 7; //随机方块

	for (int i = 0; i < 4; i++)
	{
		tile[i] = allRotationsLshape[ishape][rotation][i]; // 新方块显示为第一种旋转形态
	}

	updatetile();

	// 给新方块赋上颜色
	icolor = (starttime + rand()) % 7;//随机颜色 
	glm::vec4 newcolours[24];
	for (int i = 0; i < 24; i++)
		newcolours[i] = colors[icolor];

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// 游戏和OpenGL初始化
void init()
{
	// 游戏导语
	cout << "本游戏为俄罗斯方块，游戏开始！请先按下S键(英文输入法下)暂停游戏并阅读游戏规则。\n";
	cout << "按键操作：↑键旋转，↓键往下落，←键往左移，→键往右移，U键往上移（游戏辅助新手技能，非必要不使用）。" << endl;
	cout << "R键重启游戏，Q或ESC键退出游戏，S键暂停或继续游戏。" << endl;
	cout << "本游戏会记录积分，并根据积分逐步提升游戏难度，起始积分为 0 ，处于第 1 级难度。" << endl;

	// 初始化棋盘格，这里用画直线的方法绘制网格
	// 包含竖线 board_width+1 条
	// 包含横线 board_height+1 条
	// 一条线2个顶点坐标，并且每个顶点一个颜色值
	glm::vec4 gridpoints[board_line_num * 2];
	glm::vec4 gridcolours[board_line_num * 2];

	// 绘制网格线
	// 纵向线
	for (int i = 0; i < (board_width+1); i++)
	{
		gridpoints[2*i] = glm::vec4((tile_width + (tile_width * i)), tile_width, 0, 1);
		gridpoints[2*i + 1] = glm::vec4((tile_width + (tile_width * i)), (board_height+1) * tile_width, 0, 1);
	}

	// 水平线
	for (int i = 0; i < (board_height+1); i++)
	{
		gridpoints[ 2*(board_width+1) + 2*i ] = glm::vec4(tile_width, (tile_width + (tile_width * i)), 0, 1);
		gridpoints[ 2*(board_width+1) + 2*i + 1 ] = glm::vec4((board_width+1) * tile_width, (tile_width + (tile_width * i)), 0, 1);
	}

	// 将所有线赋成白色
	for (int i = 0; i < (board_line_num * 2); i++)
		gridcolours[i] = white;

	// 初始化棋盘格，并将没有被填充的格子设置成黑色
	glm::vec4 boardpoints[points_num];
	for (int i = 0; i < points_num; i++)
		board_colours[i] = black;

	// 对每个格子，初始化6个顶点，表示两个三角形，绘制一个正方形格子
	for (int i = 0; i < board_height; i++)
		for (int j = 0; j < board_width; j++)
		{
			glm::vec4 p1 = glm::vec4(tile_width + (j * tile_width), tile_width + (i * tile_width), .5, 1);
			glm::vec4 p2 = glm::vec4(tile_width + (j * tile_width), tile_width*2 + (i * tile_width), .5, 1);
			glm::vec4 p3 = glm::vec4(tile_width*2 + (j * tile_width), tile_width + (i * tile_width), .5, 1);
			glm::vec4 p4 = glm::vec4(tile_width*2 + (j * tile_width), tile_width*2 + (i * tile_width), .5, 1);
			boardpoints[ 6 * ( board_width * i + j ) + 0 ] = p1;
			boardpoints[ 6 * ( board_width * i + j ) + 1 ] = p2;
			boardpoints[ 6 * ( board_width * i + j ) + 2 ] = p3;
			boardpoints[ 6 * ( board_width * i + j ) + 3 ] = p2;
			boardpoints[ 6 * ( board_width * i + j ) + 4 ] = p3;
			boardpoints[ 6 * ( board_width * i + j ) + 5 ] = p4;
		}

	// 将棋盘格所有位置的填充与否都设置为false（没有被填充）
	for (int i = 0; i < board_width; i++)
		for (int j = 0; j < board_height; j++) {
			board_color[i][j] = black ; //记录相应格子的颜色
			board[i][j] = false;
		}

	// 载入着色器
	std::string vshader, fshader;
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";
	GLuint program = InitShader(vshader.c_str(), fshader.c_str());
	glUseProgram(program);

	locxsize = glGetUniformLocation(program, "xsize");
	locysize = glGetUniformLocation(program, "ysize");

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	GLuint vColor = glGetAttribLocation(program, "vColor");

	
	glGenVertexArrays(3, &vao[0]);
	glBindVertexArray(vao[0]);		// 棋盘格顶点
	
	glGenBuffers(2, vbo);

	// 棋盘格顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, (board_line_num * 2) * sizeof(glm::vec4), gridpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, (board_line_num * 2) * sizeof(glm::vec4), gridcolours, GL_STATIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	
	glBindVertexArray(vao[1]);		// 棋盘格每个格子

	glGenBuffers(2, &vbo[2]);

	// 棋盘格每个格子顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, points_num*sizeof(glm::vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格每个格子顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, points_num*sizeof(glm::vec4), board_colours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	
	glBindVertexArray(vao[2]);		// 当前方块

	glGenBuffers(2, &vbo[4]);

	// 当前方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 当前方块顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	
	glBindVertexArray(0);

	glClearColor(0, 0, 0, 0);

	// 游戏初始化
	newtile();
	starttime = glutGet(GLUT_ELAPSED_TIME);
}

// 检查在cellpos位置的格子是否被填充或者是否在棋盘格的边界范围内
bool checkvalid(glm::vec2 cellpos)
{
	//添加一个判断：格子是否已被填充
	if((board[int(cellpos.x)][int(cellpos.y)] == false&&cellpos.x >=0) && (cellpos.x < board_width) && (cellpos.y >= 0) && (cellpos.y < board_height) )
		return true;                                                                                           
	else
		return false;
}

// 在棋盘上有足够空间的情况下旋转当前方块
void rotate()
{
	// 计算得到下一个旋转方向
	int nextrotation = (rotation + 1) % 4;

	// 检查当前旋转之后的位置的有效性
	if (checkvalid((allRotationsLshape[ishape][nextrotation][0]) + tilepos)
		&& checkvalid((allRotationsLshape[ishape][nextrotation][1]) + tilepos)
		&& checkvalid((allRotationsLshape[ishape][nextrotation][2]) + tilepos)
		&& checkvalid((allRotationsLshape[ishape][nextrotation][3]) + tilepos))
	{
		// 更新旋转，将当前方块设置为旋转之后的方块
		rotation = nextrotation;
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[ishape][rotation][i];

		updatetile();
	}
}

// 检查棋盘格在row行有没有被填充满
void checkfullrow(int row)
{
	bool isfull = true;
	for (int j = 0; j < 10; j++) {
		if (board[j][row] == false) {
			isfull = false; //这一行有格子没被填满则说明不能消行
		}
	}

	// 被填满，消行，积分增加
	if (isfull == true) {
		point += 1;
		cout << "你现在已有 " << point <<" 积分"<< endl;

		// 积分数量每增加10，难度增加1，方块下落速度加快
		if (point >= hard_level * 10) {
			hard_level++;
			fall_interval /= 2; //调整下落时间间隔
			num *= 2; //与自动下落有关，使得下落时间间隔和下落数量的乘积与之前相同
			cout << "你现在所处第 " << hard_level << " 级难度" << endl;
		}

		// 处理被消除一行
		for (int j = 0; j < 10; j++) {
			changecellcolour(glm::vec2(j, row), black); //格子颜色全部变黑
			board[j][row] = false; //该行全部变成未填满状态
		}

		// 将现有全部方块下移
		for (int k = row + 1; k < 20; k++) {
			for (int j = 0; j < 10; j++) {
				if (board[j][k] == true) { //如果上一行有方块，则需要往下移一行
					board[j][k] = false; //将上一行置为未填满状态
					changecellcolour(glm::vec2(j, k - 1), board_color[j][k]);
					//下一行对应位置的颜色变成上一行对应位置的颜色
					changecellcolour(glm::vec2(j, k), black); //上一行清空为黑色
					board[j][k - 1] = true; //下一行对应位置设置为填满状态
				}
			}

		}
	}
}

// 放置当前方块，并且更新棋盘格对应位置顶点的颜色VBO
void settile()
{
	// 每个格子
	for (int i = 0; i < 4; i++)
	{
		// 获取格子在棋盘格上的坐标
		int x = (tile[i] + tilepos).x;
		int y = (tile[i] + tilepos).y;
		// 将格子对应在棋盘格上的位置设置为填充
		board[x][y] = true;
		// 并将相应位置的颜色修改
		changecellcolour(glm::vec2(x, y), colors[icolor]);//将orange改成该方块对应的颜色
	}
	for (int i = 0; i < 4; i++) {
		int y = (tile[i] + tilepos).y;
		checkfullrow(y);//这个物体的四个方块所处的行都要判断是否被填满，填满则消除并且point++
	}

}

// 给定位置(x,y)，移动方块。有效的移动值为(-1,0)，(1,0)，(0,-1)，分别对应于向
// 左，向下和向右移动。如果移动成功，返回值为true，反之为false
bool movetile(glm::vec2 direction)
{
	// 计算移动之后的方块的位置坐标
	glm::vec2 newtilepos[4];
	for (int i = 0; i < 4; i++)
		newtilepos[i] = tile[i] + tilepos + direction;

	// 检查移动之后的有效性
	if (checkvalid(newtilepos[0])
		&& checkvalid(newtilepos[1])
		&& checkvalid(newtilepos[2])
		&& checkvalid(newtilepos[3]))
		{
			// 有效：移动该方块
			tilepos.x = tilepos.x + direction.x;
			tilepos.y = tilepos.y + direction.y;

			updatetile();

			return true;
		}

	return false;
}

// 重新启动游戏
void restart()
{
	// 先将所有格子清空
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 20; j++) {
			board[i][j] = false; // 所有格子填充状态设为false
			board_color[i][j] = black; // 所有格子颜色设为黑色
			changecellcolour(glm::vec2(i, j), black);
		}
	}

	starttime = glutGet(GLUT_ELAPSED_TIME);
	hard_level = 1;
	fall_interval = 1000; //难度和下落时间重新设立
	num = 1; //初始化
	gamestop = false; //设置游戏为运行状态
	gameover = false; //结束游戏的标志置为false
	point = 0; //积分清零
	cout << "您已重启游戏，积分已清零，难度已恢复为1" << endl;
	cout << "你现在已有 " << point << " 积分，" << "处于第 " << hard_level << " 级难度" << endl;
	newtile();
}

//方块自动下落
void falling(void)
{
	//falling函数会不断地执行，时间也会不断地增加
	current_time = glutGet(GLUT_ELAPSED_TIME);

	// 由于时间不断的增加，我们用时间是否大于num乘以fall_interval来判断是否要进行下落操作
	// 从而实现每 fall_interval 的时间间隔方块自动向下掉落，随着难度的增加fall_interval会越来越小，方块下落速度越来越快
	if (current_time - starttime > num * fall_interval) {
		num++;
		if (gameover == false && gamestop == false) {//当游戏还在进行并且没有暂停时
			if (!movetile(glm::vec2(0, -1)))//自动下落
			{
				settile();
				newtile();
			}
		}
	}
}

// 游戏渲染部分
void display()
{
	falling();
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize);
	glUniform1i(locysize, ysize);

	glBindVertexArray(vao[1]);
	glDrawArrays(GL_TRIANGLES, 0, points_num); // 绘制棋盘格 (width * height * 2 个三角形)
	glBindVertexArray(vao[2]);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制当前方块 (8 个三角形)
	glBindVertexArray(vao[0]);
	glDrawArrays(GL_LINES, 0, board_line_num * 2 );		 // 绘制棋盘格的线

}

// 在窗口被拉伸的时候，控制棋盘格的大小，使之保持固定的比例。
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

// 键盘响应事件中的特殊按键响应
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if(!gameover)
	{
		switch(key)
		{	
			// 控制方块的移动方向，更改形态
			case GLFW_KEY_UP:	// 向上按键旋转方块
				if ((action == GLFW_PRESS || action == GLFW_REPEAT)&&gamestop==false)
				{
					rotate();
					break;
				}
				else
				{
					break;
				}
			case GLFW_KEY_DOWN: // 向下按键移动方块
				if ((action == GLFW_PRESS || action == GLFW_REPEAT) && gamestop == false){
					if (!movetile(glm::vec2(0, -1)))
					{
						settile();
						newtile();
						break;
					}
					else
					{
						break;
					}
				}
			case GLFW_KEY_LEFT:  // 向左按键移动方块
				if ((action == GLFW_PRESS || action == GLFW_REPEAT) && gamestop == false){
					movetile(glm::vec2(-1, 0));
					break;
				}
				else
				{
					break;
				}
			case GLFW_KEY_RIGHT: // 向右按键移动方块
				if ((action == GLFW_PRESS || action == GLFW_REPEAT) && gamestop == false){
					movetile(glm::vec2(1, 0));
					break;
				}
				else
				{
					break;
				}
			// 游戏设置。
			case GLFW_KEY_ESCAPE:
				if(action == GLFW_PRESS){
					cout << "您已退出游戏，欢迎下次再来玩。" << endl;
					exit(EXIT_SUCCESS);
					break;
				}
				else
				{
					break;
				}
			case GLFW_KEY_Q:
				if(action == GLFW_PRESS){
					cout << "您已退出游戏，欢迎下次再来玩。" << endl;
					exit(EXIT_SUCCESS);
					break;
				}
				else
				{
					break;
				}
				
			case GLFW_KEY_R:
				if(action == GLFW_PRESS){

					restart();
					break;
				}
				else
				{
					break;
				}	
			
			case  GLFW_KEY_U: // U键向上移动方块
				if (action == GLFW_PRESS) {
					movetile(glm::vec2(0, 1));
					break;
				}
				else
				{
					break;
				}

			case GLFW_KEY_S: //实现游戏的暂停
				if (action == GLFW_PRESS) {
					if (gamestop == false) {
						gamestop = true;
						cout << "你已暂停游戏!" << endl;
					}
					else {
						gamestop = false;
						cout << "游戏继续!" << endl;
					}
					break;
				}
				else
				{
					break;
				}
		}
	}
	else {
	switch (key) {
	case GLFW_KEY_R:
		if (action == GLFW_PRESS) {
			restart();
			break;
		}
		else
		{
			break;
		}

		}
}
}


int main(int argc, char **argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	// 创建窗口。
	SetConsoleOutputCP(CP_UTF8);
	GLFWwindow* window = glfwCreateWindow(500, 900, "name", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
	
	
	init();
	while (!glfwWindowShouldClose(window))
    { 
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();	
    }
    glfwTerminate();
    return 0;
}
