#include "usrGameController.h"
#include "qtcyberdip.h"
#include <cmath>
#include <iostream>
#include <Windows.h>
#ifdef VIA_OPENCV
using namespace std;
//构造与初始化
usrGameController::usrGameController(void* qtCD)
{
	qDebug() << "usrGameController online.";
	device = new deviceCyberDip(qtCD);//设备代理类
	cv::namedWindow(WIN_NAME);
	cv::setMouseCallback(WIN_NAME, mouseCallback, (void*)&(argM));
}

//析构
usrGameController::~usrGameController()
{
	cv::destroyAllWindows();
	if (device != nullptr)
	{
		delete device;
	}
	qDebug() << "usrGameController offline.";
}

//处理图像 
int usrGameController::usrProcessImage(cv::Mat& img)
{
	cv::Size imgSize(542,722);
	if (imgSize.height <= 0 || imgSize.width <= 0)
	{
		qDebug() << "Invalid image. Size:" << imgSize.width << "x" << imgSize.height;
		return -1;
	}

	//截取图像边缘（原始图）
	cv::Mat pt = img(cv::Rect(0, UP_CUT, 542,722));

	//灰度图
	cv::Mat gray_image;
	cvtColor(pt, gray_image, CV_BGR2GRAY);

	cv::imshow(WIN_NAME, gray_image);

	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	// 得到二值图
	cv::threshold(pt, pt, 100, 255, CV_THRESH_BINARY);
	//cv::imshow(WIN_NAME, pt);

	cv::Mat result(pt.size(), CV_8U, cv::Scalar(255));
	cv::Mat_ <cv::Vec3b>::const_iterator it = pt.begin<cv::Vec3b>();
	cv::Mat_ <cv::Vec3b>::const_iterator itend = pt.end<cv::Vec3b>();
	cv::Mat_ <uchar>::iterator itout = result.begin<uchar>();


	for (; it != itend; ++it, ++itout)
	{
		if (getColor(*it))
		{
			*itout = 0;
		}
		else
		{
			*itout = 255;
		}
	}

	int i, j;

	for (i = 0; i < 22; ++i)
		for (j = 0; j < 10; ++j)
		{
			t[10 * i + j] = result.at<uchar>(115 + 21.5 * i, 100 + 21.5 * j);
		}

	for (i = 0; i < 22; ++i)
		for (j = 0; j < 10; ++j)
		{
			if (t[10 * i + j] == 255)
				board[21 - i][j] = 1;
			else board[21 - i][j] = 0;
		}

	//Sleep(400);
	//qDebug() << move << rotate;
	get_input_cube_type();
	if(input_cube_type != 7)
	{
		game_strategy();
		qtCyberDIPcontrol();
		Sleep(400);
		qDebug() << "move:"<<move << "rotate:"<<rotate;
	}

	
	//判断鼠标点击尺寸
	if (argM.box.x >= 0 && argM.box.x < imgSize.width &&
		argM.box.y >= 0 && argM.box.y < imgSize.height
		)
	{
		
		qDebug() << "X:" << argM.box.x << " Y:" << argM.box.y;
		device->comHitDown();
	
		
		//for (i = 0; i < 22; i++)
		//{
		//	for (j = 0; j < 10; j++)
		//		qDebug() << i + 1 << "hang" << j + 1 << "lie" << board[i][j];
		//}

		if (argM.Hit)
		{
			device->comHitDown();
		}
		device->comMoveToScale(((double)argM.box.x + argM.box.width) / pt.cols, ((double)argM.box.y + argM.box.height) / pt.rows);
		argM.box.x = -1; argM.box.y = -1;
		if (argM.Hit)
		{
			device->comHitUp();
		}
		else
		{
			device->comHitOnce();
		}
	}
	return 0;
}

bool usrGameController::getColor(const cv::Vec3b &color)
{
	return abs(color[0]) < 10 && abs(color[1]) < 10 && abs(color[2]) < 10;
}
//游戏控制部分
//就是函数 game strategy 他需要的输入是 由游戏图像得到的矩阵 一个游戏棋盘的矩阵
//22 * 10的  需要截的图片是 上面的物块刚落下时 到第四行的图像  其他的不需要
//输出的两个数 move是左右移动 左负右正 rotate是旋转次数

void usrGameController::get_input_cube_type()
{
	if (board[19][5] == 1 && board[19][6] == 1 && board[19][7] == 1 && board[18][6] == 1)input_cube_type = 0;
	else if (board[18][5] == 1 && board[20][6] == 1 && board[19][6] == 1 && board[18][6] == 1)input_cube_type = 1;
	else if (board[19][5] == 1 && board[19][6] == 1 && board[18][5] == 1 && board[18][6] == 1)input_cube_type = 2;
	else if (board[19][6] == 1 && board[20][6] == 1 && board[19][5] == 1 && board[18][5] == 1)input_cube_type = 3;
	else if (board[20][5] == 1 && board[19][5] == 1 && board[18][5] == 1 && board[18][6] == 1)input_cube_type = 4;
	else if (board[21][5] == 1 && board[20][5] == 1 && board[19][5] == 1 && board[18][5] == 1)input_cube_type = 5;
	else if (board[20][5] == 1 && board[19][5] == 1 && board[19][6] == 1 && board[18][6] == 1)input_cube_type = 6;
	else input_cube_type = 7;
}

void usrGameController::game_strategy()
{
	int landing_height, row_eroded, row_transition, col_transition, buried_holes, wells, row_eroded_drop = 0, row_size = 0, row_size_drop = 0, mov = 0, rotat = 0;
	double a1 = -4.5, a2 = 3.518, a3 = -3.218, a4 = -9.349, a5 = -7.899, a6 = -3.386;
	double value = 0, value_c = -1000;
	int board_drop[22][10] = { 0 };
	int board_result[22][10] = { 0 };
	move = 0; rotate = 0;


	for (int i = 0; i < 22; i++)
	{
		bool flag = 0;
		for (int j = 0; j < 10; j++)
		{
			if (board[i][j] == 1)flag = 1;
			board_drop[i][j] = board[i][j];
		}
		if (!flag) { row_size = i+1; break; }
	}
	qDebug() << "size:"<<row_size<<"type:"<<input_cube_type;

	if (row_size == 0)
	{
		move = -5; rotate = 0;
	}
	else
	{
		if (input_cube_type == 2)
		{
			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--) { if (board[r][c] == 1 || board[r][c + 1] == 1) { landing_height = r + 1; break; } }
				board_drop[landing_height][c] = 1;
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height][c + 1] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 2;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height][c + 1] = 0;
			}
			move = mov; rotate = rotat;
		}
		else if (input_cube_type == 7)
		{
			move = 0;
			rotate = 0;
		}
		else if (input_cube_type == 5)
		{
			for (int c = 0; c < 7; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1 || board[r][c + 2] == 1 || board[r][c + 3] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height][c + 2] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height][c + 3] = 1;
				if (landing_height + 1 > row_size) { row_size_drop = landing_height + 1; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 4;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 3; }//OK
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 2] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height][c + 3] = 0;
			}

			for (int c = 0; c < 10; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height + 2][c] = 1;
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height + 3][c] = 1;
				if (landing_height + 4 > row_size) { row_size_drop = landing_height + 4; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; }//OK
				board_drop[landing_height][c] = 0;
				board_drop[landing_height + 2][c] = 0;
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height + 3][c] = 0;
			}
			move = mov; rotate = rotat;
		}

		else if (input_cube_type == 6)
		{
			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1 || board[r + 1][c + 2] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 1][c + 2] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 2;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 1; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 1][c + 2] = 0;
			}

			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size); r > 0; r--)
				{
					if (board[r][c] == 1 || board[r - 1][c + 1] == 1) { landing_height = r; break; }
				}
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 2][c] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1.5;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; rotat = 0; }
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 2][c] = 0;
			}
			move = mov; rotate = rotat;
		}
		else if (input_cube_type == 3)//ok 
		{
			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r - 1][c + 1] == 1 || board[r - 1][c + 2] == 1) { landing_height = r; break; }
				}
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height][c + 2] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 1][c] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 2;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 1; }//OK
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height][c + 2] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 1][c] = 0; 
			}

			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r + 1][c + 1] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height][c] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 2][c + 1] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1.5;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; rotat = 0; }
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height][c] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 2][c + 1] = 0;
			}
			move = mov; rotate = rotat;
		}
		else if (input_cube_type == 4)
		{
			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size + 1); r > 1; r--)
				{
					if (board[r][c] == 1 || board[r - 2][c + 1] == 1) { landing_height = r - 1; break; }
				}
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 2][c] = 1;
				board_drop[landing_height + 2][c + 1] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1.33;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 2; }//已OK
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 2][c] = 0;
				board_drop[landing_height + 2][c + 1] = 0;
			}

			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1 || board[r][c + 2] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height][c + 2] = 1;
				board_drop[landing_height + 1][c + 2] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 3;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c -4; rotat = 3; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height][c + 2] = 0;
				board_drop[landing_height + 1][c + 2] = 0;
			}

			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height + 2][c] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 2;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; rotat = 0; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height + 2][c] = 0;
			}

			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r + 1][c + 1] == 1 || board[r + 1][c + 2] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height + 1][c + 2] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 2;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 1; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height + 1][c + 2] = 0;
			}
			move = mov; rotate = rotat;
		}

		else if (input_cube_type == 1)
		{
			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r + 2][c + 1] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height][c] = 1;
				board_drop[landing_height + 2][c] = 1;
				board_drop[landing_height + 2][c + 1] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1.33;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 2; }//OK
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height][c] = 0;
				board_drop[landing_height + 2][c] = 0;
				board_drop[landing_height + 2][c + 1] = 0;
			}

			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size); r > 0; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1 || board[r - 1][c + 2] == 1) { landing_height = r; break; }
				}
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 1][c + 2] = 1;
				board_drop[landing_height][c + 2] = 1;
				if (landing_height + 1 > row_size) { row_size_drop = landing_height + 1; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 2;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 3; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height][c + 2] = 0;
				board_drop[landing_height - 1][c + 2] = 0;
			}

			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 2][c + 1] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1.33;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; rotat = 0; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 2][c + 1] = 0;
			}

			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1 || board[r][c + 2] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height][c + 2] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 3;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 1; }
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height][c + 2] = 0;
			}
			move = mov; rotate = rotat;
		}

		else if (input_cube_type == 0)
		{
			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size); r > 0; r--)
				{
					if (board[r][c] == 1 || board[r - 1][c + 1] == 1 || board[r][c + 2] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 1][c + 2] = 1;
				board_drop[landing_height + 1][c] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 2;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; rotat = 0; }
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 1][c + 2] = 0;
				board_drop[landing_height + 1][c] = 0;
			}

			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size); r > 0; r--)
				{
					if (board[r][c] == 1 || board[r - 1][c + 1] == 1) { landing_height = r; break; }
				}
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				board_drop[landing_height + 2][c + 1] = 1;
				board_drop[landing_height + 1][c] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1.5;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 1; }//OK
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
				board_drop[landing_height + 2][c + 1] = 0;
				board_drop[landing_height + 1][c] = 0;
			}

			for (int c = 0; c < 8; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r][c + 1] == 1 || board[r][c + 2] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height][c + 1] = 1;
				board_drop[landing_height][c + 2] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				if (landing_height + 2 > row_size) { row_size_drop = landing_height + 2; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 3;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 4; rotat = 2; }//OK
				board_drop[landing_height][c] = 0;
				board_drop[landing_height][c + 1] = 0;
				board_drop[landing_height][c + 2] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
			}

			for (int c = 0; c < 9; c++)
			{
				landing_height = 0;
				for (int r = (row_size - 1); r > -1; r--)
				{
					if (board[r][c] == 1 || board[r + 1][c + 1] == 1) { landing_height = r + 1; break; }
				}
				board_drop[landing_height][c] = 1;
				board_drop[landing_height + 1][c] = 1;
				board_drop[landing_height + 2][c] = 1;
				board_drop[landing_height + 1][c + 1] = 1;
				if (landing_height + 3 > row_size) { row_size_drop = landing_height + 3; }
				else { row_size_drop = row_size; }
				matrix(board_result, board_drop, row_size_drop, &row_eroded_drop);
				row_eroded = row_eroded_drop * 1.5;
				row_transition = count_row_transition(board_result, row_size_drop - row_eroded_drop);
				col_transition = count_col_transition(board_result, row_size_drop - row_eroded_drop);
				buried_holes = count_buried_holes(board_result, row_size_drop - row_eroded_drop);
				wells = count_wells(board_result, row_size_drop - row_eroded_drop);
				value = a1 * landing_height + a2 * row_eroded + a3 * row_transition + a4 * col_transition + a5 * buried_holes + a6 * wells;
				if (value >= value_c) { value_c = value; mov = c - 5; rotat = 3; }//OK
				board_drop[landing_height][c] = 0;
				board_drop[landing_height + 1][c] = 0;
				board_drop[landing_height + 2][c] = 0;
				board_drop[landing_height + 1][c + 1] = 0;
			}
			move = mov; rotate = rotat;
		}

	}

}
void usrGameController::matrix(int matrix_s[][10], int matrix[][10], int  r_size, int *numL)
{
	*numL = 0;
	int rr;
	for (int ii = 0; ii < 20; ii++)
	{
		for (int kk = 0; kk < 10; kk++) { matrix_s[ii][kk] = matrix[ii][kk]; }
	}
	rr = r_size;
	for (int i = 0; i < rr; i++)
	{
		bool flag = 1;
		for (int f = 0; f < 10; f++) { if (matrix_s[i][f] == 0)flag = 0; }
		if (flag)
		{
			*numL = *numL + 1;
			for (int j = i; j < rr; j++)
			{
				for (int ii = 0; ii < 10; ii++) { matrix_s[j][ii] = matrix_s[j + 1][ii]; }
			}
			rr = rr - 1;
			i = i - 1;
		}
	}
}
int usrGameController::count_row_transition(int matrix[][10], int r_size)
{
	int RT_num = 0;
	for (int i = 0; i < r_size; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (matrix[i][j] - matrix[i][j + 1] != 0) { RT_num += 1; }
		}
		if (matrix[i][0] == 0) { RT_num += 1; }
		if (matrix[i][9] == 0) { RT_num += 1; }
	}
	return RT_num;
}
int usrGameController::count_col_transition(int matrix[][10], int r_size)
{
	int CT_num = 0;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < r_size; j++)
		{
			if (matrix[j][i] - matrix[j + 1][i] != 0) { CT_num += 1; }
		}
		if (matrix[0][i] == 0) { CT_num += 1; }
	}
	return CT_num;
}

int usrGameController::count_buried_holes(int matrix[][10], int r_size)
{
	int NH_num = 0;
	for (int i = 0; i < 10; i++)
	{
		int roof = 0;
		for (int j = r_size - 1; j > -1; j--)
		{
			if (matrix[j][i] == 1) { roof = j; break; }
		}
		if (roof != 0)
		{
			for (int j = 0; j < roof; j++)
			{
				if (matrix[j][i] == 0)NH_num += 1;
			}
		}
	}
	return NH_num;
}

int usrGameController::count_wells(int matrix[][10], int r_size)
{
	int Well_sum = 0, Well_depth = 0;
	for (int i = 1; i < 9; i++)
	{
		for (int j = r_size - 1; j > -1; j--)
		{
			if (matrix[j][i] == 0 && matrix[j][i + 1] == 1 && matrix[j][i - 1] == 1) { Well_depth += 1; }
			else if (matrix[j][i] == 1) { Well_sum = Well_sum + 0.5*Well_depth*(Well_depth + 1);  Well_depth = 0; }
			if (j == 0) { Well_sum = Well_sum + 0.5*Well_depth*(Well_depth + 1);  Well_depth = 0; }
		}
		//cout<<i<<' '<<Well_sum<<endl;
	}
	for (int j = r_size - 1; j > -1; j--)
	{
		if (matrix[j][0] == 0 && matrix[j][1] == 1) { Well_depth += 1; }
		if (matrix[j][0] == 1 || j == 0) { Well_sum = Well_sum + 0.5*Well_depth*(Well_depth + 1);  Well_depth = 0; }
	}
	//cout<<"0 "<<Well_sum<<endl;
	for (int j = r_size - 1; j > -1; j--)
	{
		if (matrix[j][9] == 0 && matrix[j][8] == 1) { Well_depth += 1; }
		if (matrix[j][9] == 1 || j == 0) { Well_sum = Well_sum + 0.5*Well_depth*(Well_depth + 1);  Well_depth = 0; }
	}
	//cout<<"9 "<<Well_sum<<endl;
	return Well_sum;
}

//qtCyberDIP控制(d=move, a = ;i=rotate, 
void usrGameController::qtCyberDIPcontrol()
{
	int na = 0, nc = 0, nr =0 , a = 0;
	int delay = 300;
	if (move > 0) { a = 0; }
	if (move == 0) { a = 1; }
	if (move < 0) { a = 2; }
	
	switch (rotate) {

	case 0:
		//device->comDeviceDelay(delay);
		Sleep(delay);
		break;

	case 1:
		device->comMoveTo(5, 40);
		//device->comDeviceDelay(delay);
		Sleep(delay);
		device->comHitOnce();
		//device->comDeviceDelay(delay);
		Sleep(delay);
		break;

	case 2:
		device->comMoveTo(5, 40);
		//device->comDeviceDelay(delay);
		Sleep(delay);
		device->comHitOnce();
		//device->comDeviceDelay(delay);
		Sleep(delay);
		device->comHitOnce();
		//device->comDeviceDelay(delay);
		Sleep(delay);
		break;

	case 3:
		device->comMoveTo(5, 40);
		//device->comDeviceDelay(delay);
		Sleep(delay);
		device->comHitOnce();
		//device->comDeviceDelay(delay);
		Sleep(delay);
		device->comHitOnce();
		//device->comDeviceDelay(delay);
		Sleep(delay);
		device->comHitOnce();
		//device->comDeviceDelay(delay);
		Sleep(delay);
		break;
	}

	switch (a) {

	case 0:
		device->comMoveTo(5,30);
		//device->comDeviceDelay(10);
		Sleep(delay);
		for (na = 0; na < abs(move); na++) {
			device->comHitOnce();
			//device->comDeviceDelay(delay);
			Sleep(delay);
		};

		break;

	case 1:
		//device->comMoveTo(20,20);
		//device->comDeviceDelay(delay);
		Sleep(delay);
		break;

	case 2:
		device->comMoveTo(5,15);
		//device->comDeviceDelay(delay);
		Sleep(delay);
		for (nc = 0; nc < abs(move); nc++) {
			device->comHitOnce();
			//device->comDeviceDelay(delay);
			Sleep(delay);

		}
	}
}

//鼠标回调函数
void mouseCallback(int event, int x, int y, int flags, void*param)
{
	usrGameController::MouseArgs* m_arg = (usrGameController::MouseArgs*)param;
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE: // 鼠标移动时
	{
		if (m_arg->Drawing)
		{
			m_arg->box.width = x - m_arg->box.x;
			m_arg->box.height = y - m_arg->box.y;
		}
	}
	break;
	case CV_EVENT_LBUTTONDOWN:case CV_EVENT_RBUTTONDOWN: // 左/右键按下
	{
		m_arg->Hit = event == CV_EVENT_RBUTTONDOWN;
		m_arg->Drawing = true;
		m_arg->box = cvRect(x, y, 0, 0);
	}
	break;
	case CV_EVENT_LBUTTONUP:case CV_EVENT_RBUTTONUP: // 左/右键弹起
	{
		m_arg->Hit = false;
		m_arg->Drawing = false;
		if (m_arg->box.width < 0)
		{
			m_arg->box.x += m_arg->box.width;
			m_arg->box.width *= -1;
		}
		if (m_arg->box.height < 0)
		{
			m_arg->box.y += m_arg->box.height;
			m_arg->box.height *= -1;
		}
	}
	break;
	}
}
#endif