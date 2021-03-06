#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include "format.h"

#define ERR_FILE_OPEN_FAILED   -1
#define ERR_FILE_SIZE_MISMATCH -2
#define ERR_FILE_READ_FAILED   -3
#define ERR_MEM_ALLOC_FAILED   -4
#define ERR_UNKNOWN_FORMAT     -5

#define MODE_1 1



#define SHOW_PLAN_TYTLE
#define SHOW_LINE_NUM
#define MAX_PLAN 3
#define DEFAULT_DATA_W 16
#define DEFAULT_DATA_H 16
#define PLAN_TYTLE_LENGTH 16
#define STATUS_STRING_LENGTH 128
#define LINE_NUM_LENGTH 4
#define H_BLANK 2
typedef struct {
	int showTytle;
	int showLineNum;
    int hideTop;
    int hideLeft;
	int x;
	int y;
	int w;
	int h;
	int tytle_x;
	int tytle_y;
	int line_num_x;
	int line_num_y;
	int data_x;
	int data_y;
	int data_w;
	int data_h;
	int read_x;
	int read_y;
	int plan_w;
	int plan_h;
    int highlight_pos_x;
    int highlight_pos_y;
	char tytle[PLAN_TYTLE_LENGTH+1];
	unsigned int *data;
} PLAN_WINDOW;

typedef struct {
	int scr_lines;
	int scr_cols;
	int status_bar_x;
	int status_bar_y;
	char status_str[STATUS_STRING_LENGTH+1];
	PLAN_WINDOW plann[MAX_PLAN];
} WINDOW_CONFIG;

/* input param */
int CIF_WIDTH = 0;
int CIF_HEIGHT = 0;
int CIF_FORMAT = FMT_YUV422P;
int CIF_DEC_MODE = 0;
int CIF_OUTPUT = 0;
char CIF_OUTPUT_FILE_NAME[1024];
int CIF_OUTPUT_SINGLE_PLAN = 0;
int CIF_OUTPUT_PLAN = 0;

WINDOW_CONFIG winCfg;
FRAME_INFO frameInfo;



int get_plan_data_offset_x(int x_idx, int end)
{
    if (CIF_DEC_MODE)
    {
        if (end)
        {
            return (x_idx + 1) * 4 + ((x_idx >> 4) * H_BLANK);
        }
        else
        {
            return x_idx * 4 + ((x_idx >> 4) * H_BLANK);
        }
    }
    else
    {
        if (end)
        {
            return (x_idx + 1) * 4 + ((x_idx >> 4) * H_BLANK);
        }
        else
        {
            return x_idx * 4 + ((x_idx >> 4) * H_BLANK);
        }
    }
}

int get_plan_data_offset_y(int y_idx, int end)
{
    if (end)
    {
        return y_idx;
    }
    else
    {
        return y_idx;
    }
}

int get_plan_w(int data_w, int showLineNum, int showTytle, int hideTop, int hideLeft)
{
	int plan_w = 0;
	plan_w = get_plan_data_offset_x(data_w - 1, 1)+3;
    if (hideLeft)
    {
        plan_w -= 1;
    }
	if (showLineNum)
	{
		plan_w += (LINE_NUM_LENGTH + 2);
	}
	return plan_w;
}

int get_plan_h(int data_h, int showLineNum, int showTytle, int hideTop, int hideLeft)
{
	int plan_h = 0;
	plan_h = get_plan_data_offset_y(data_h - 1, 1)+3;
    if (hideTop)
    {
        plan_h -= 1;
    }
	if (showTytle)
	{
		plan_h += 2;
	}
	return plan_h;
}

void draw_progress_bar(int x, int y, int length, float percentage, int v)
{
    int i;
    int t = length * percentage;
    if (v) {
        for (i = 0; i < length; i++) {
            move(y+i, x);
            addch(i == t ? '#' : '|');
        }
    } else {
        move(y, x);
        for (i = 0; i < length; i++) {
            addch(i == t ? '#' : '-');
        }
    }
}

void RDWIN_DrawPlannBorder(PLAN_WINDOW *window)
{
#if 1
    #define MY_ACS_ULCORNER '-'
    #define MY_ACS_URCORNER '-'
    #define MY_ACS_LLCORNER '-'
    #define MY_ACS_LRCORNER '-'
    #define MY_ACS_HLINE    '-'
    #define MY_ACS_VLINE    '|'
    #define MY_ACS_LTEE     '|'
    #define MY_ACS_RTEE     '|'
#else
    #define MY_ACS_ULCORNER ACS_ULCORNER
    #define MY_ACS_URCORNER ACS_URCORNER
    #define MY_ACS_LLCORNER ACS_LLCORNER
    #define MY_ACS_LRCORNER ACS_LRCORNER
    #define MY_ACS_HLINE    ACS_HLINE   
    #define MY_ACS_VLINE    ACS_VLINE   
    #define MY_ACS_LTEE     ACS_LTEE    
    #define MY_ACS_RTEE     ACS_RTEE    
#endif
	int i;
	int cur_x = 0;
	int cur_y = 0;
	
	cur_x = window->x;
	cur_y = window->y;
	move(cur_y, cur_x);
    if (!window->hideTop)
    {
        if (!window->hideLeft) {
            cur_x++;
            addch(MY_ACS_ULCORNER); //"┌"
        }
        for (; cur_x < window->x + window->w-1; cur_x++) {
            addch(MY_ACS_HLINE); //"─"
        }
        addch(MY_ACS_URCORNER); //"┐"
        cur_y++;
        cur_x = window->x;
    }
	
	if (window->showTytle)
	{
		window->tytle_y = cur_y;
		move(cur_y, cur_x);
        if (!window->hideLeft) {
            addch(MY_ACS_VLINE);
            cur_x ++;
        }
        window->tytle_x = cur_x+1; // tytle position
		cur_x = window->x + window->w-1;
		move(cur_y, cur_x);
		addch(MY_ACS_VLINE);
		cur_y++;

        cur_x = window->x;
		move(cur_y, cur_x);
        if (!window->hideLeft) {
            cur_x++;
            addch(MY_ACS_LTEE);
        }
		for (; cur_x < window->x + window->w-1; cur_x++) {
			addch(MY_ACS_HLINE); //"─"
		}
		addch(MY_ACS_RTEE);
		cur_x = window->x;
		cur_y++;
	}
	
    if (window->hideLeft) {
        window->line_num_x = cur_x+1;
        window->data_x = cur_x+1;
    } else {
        window->line_num_x = cur_x+2;
        window->data_x = cur_x+2;
    }
	if (window->showLineNum)
	{
		window->data_x += LINE_NUM_LENGTH + 2;
	}
	window->line_num_y = cur_y;
	window->data_y = cur_y;
	for (i = cur_y; i < window->y + window->h - 1; i++) {
		move(cur_y, cur_x);
        if (!window->hideLeft) {
            cur_x += 1;
            addch(MY_ACS_VLINE); //"│"
        }
		cur_x = window->x + window->w-1;
		move(cur_y, cur_x);
		addch(MY_ACS_VLINE); //"│"
		cur_x = window->x;
		cur_y++;
	}
	
	move(cur_y, cur_x);
    if (!window->hideLeft) {
        cur_x++;
        addch(MY_ACS_LLCORNER); //"└"
    }
	for (; cur_x < window->x + window->w-1; cur_x++) {
		addch(MY_ACS_HLINE); //"─"
	}
	addch(MY_ACS_LRCORNER); //"┘"
}

void RDWIN_DrawPlannData(PLAN_WINDOW *window)
{
	int x,y;
	unsigned char byte;
	char hexstr[3] = {0};
	move(window->data_y, window->data_x);
	for (y = 0; y < window->data_h; y++) {
		for (x = 0; x < window->data_w; x++) {
			byte = window->data[y * window->data_w + x];
            if (CIF_DEC_MODE)
            {
                sprintf(hexstr, "%-3d", byte);
            }
            else
            {
			    sprintf(hexstr, "%02x", byte);
            }
			move(window->data_y+get_plan_data_offset_y(y, 0), window->data_x+get_plan_data_offset_x(x, 0));
            if (window->highlight_pos_x == x && window->highlight_pos_y == y)
                standout();
			addstr(hexstr);
            if (window->highlight_pos_x == x && window->highlight_pos_y == y)
                standend();
		}
	}
}

void RDWIN_DrawLineNum(PLAN_WINDOW *window)
{
	int y;
	int num;
	char hexstr[5] = {0};
	if (window->showLineNum)
	{
		move(window->data_y, window->data_x);
		for (y = 0; y < window->data_h; y++) {
			num = y + window->read_y;
			sprintf(hexstr, "%04d", num);
			move(window->line_num_y+get_plan_data_offset_y(y, 0),window->line_num_x);
			addstr(hexstr);
		}
	}
}

void RDWIN_DrawTytle(PLAN_WINDOW *window)
{
	if (window->showTytle)
	{
		move(window->tytle_y, window->tytle_x);
		addstr(window->tytle);
	}
}

void RDWIN_ReadPlannData(PLAN_WINDOW *window, FRAME_PLAN *frame, int x, int y)
{
	int i,j;
	for (i = 0; i < window->data_h; i++)
	{
		for (j = 0; j < window->data_w; j++)
		{
			window->data[i * window->data_w + j] = read_pos(frame, x + j, y + i);
		}
	}
}

void RDWIN_ClearStatus(WINDOW_CONFIG *cfg)
{
    int i;
    move(cfg->status_bar_y, cfg->status_bar_x);
	for (i = 0; i < strlen(cfg->status_str); i++)
    {
        addch(' ');
    }
}

void RDWIN_DrawStatus(WINDOW_CONFIG *cfg)
{
    RDWIN_ClearStatus(cfg);
    move(cfg->status_bar_y, cfg->status_bar_x);
	snprintf(cfg->status_str, STATUS_STRING_LENGTH, "%d,%d",
        cfg->plann[0].read_y + cfg->plann[0].highlight_pos_y,
        cfg->plann[0].read_x + cfg->plann[0].highlight_pos_x);
    addstr(cfg->status_str);
}

void RDWIN_DrawProgressBar(WINDOW_CONFIG *cfg)
{
    float per = 0.0f;
    per = (float)cfg->plann[0].read_y / CIF_HEIGHT;
    draw_progress_bar(cfg->plann[0].w+1, 1, cfg->status_bar_y-1, per, 1);
	per = (float)cfg->plann[0].read_x / CIF_WIDTH;
    draw_progress_bar(10, cfg->status_bar_y, cfg->plann[0].w-10, per, 0);
}

int RDWIN_Init(int mode)
{
	initscr();
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr,FALSE);
	keypad(stdscr,TRUE);
	clear();
	
	int i;
	int cur_x = 0;
	int cur_y = 0;
	winCfg.scr_lines = LINES;
	winCfg.scr_cols = COLS;
	int x_off_list[MAX_PLAN] = {0, 0, -1};
	int y_off_list[MAX_PLAN] = {0, -1, -1};
    int w_list[MAX_PLAN] = {DEFAULT_DATA_W, DEFAULT_DATA_W / 2, DEFAULT_DATA_W / 2};
    int h_list[MAX_PLAN] = {DEFAULT_DATA_H, DEFAULT_DATA_H, DEFAULT_DATA_H};
	if (COLS >= 61)
	{
		w_list[0] = 16;
	}
	else if (COLS >= 35)
	{
		w_list[0] = 8;
	}
	else
	{
		w_list[0] = 4;
	}
	w_list[1] = w_list[2] = w_list[0] / 2;
	
	if (CIF_FORMAT == FMT_YUV422P || CIF_FORMAT == FMT_YUV422I)
	{
		if (LINES >= 41)
		{
			h_list[0] = h_list[1] = h_list[2] = 16;
		}
		else if (LINES >= 26)
		{
			h_list[0] = h_list[1] = h_list[2] = 8;
		}
		else if (LINES >= 17)
		{
			h_list[0] = h_list[1] = h_list[2] = 4;
		}
	}
	else if (CIF_FORMAT == FMT_YUV420P || CIF_FORMAT == FMT_YUV420SP)
	{
		if (LINES >= 32)
		{
			h_list[0] = 16;
		}
		else if (LINES >= 20)
		{
			h_list[0] = 8;
		}
		else
		{
			h_list[0] = 4;
		}
		h_list[1] = h_list[2] = h_list[0] / 2;
	}
	int new_line[MAX_PLAN] = {1, 0, 1};
	int showTytle[MAX_PLAN] = {1, 1, 1};
	int showLineNum[MAX_PLAN] = {1, 1, 0};
    int hideTop[MAX_PLAN] = {0, 0, 0};
    int hideLeft[MAX_PLAN] = {0, 0, 0};
	char *tytle[MAX_PLAN] = {"Y", "U", "V"};
	
	for (i = 0; i < MAX_PLAN; i++)
	{
		winCfg.plann[i].showTytle = showTytle[i];
		winCfg.plann[i].showLineNum = showLineNum[i];
        winCfg.plann[i].hideTop = hideTop[i];
        winCfg.plann[i].hideLeft = hideLeft[i];
		winCfg.plann[i].data_w = w_list[i];
		winCfg.plann[i].data_h = h_list[i];
        winCfg.plann[i].data = malloc(w_list[i] * h_list[i] * sizeof(winCfg.plann[i].data[0]));
        if (NULL == winCfg.plann[i].data)
        {
            return ERR_MEM_ALLOC_FAILED;
        }
        winCfg.plann[i].highlight_pos_x = 0;
        winCfg.plann[i].highlight_pos_y = 0;
		winCfg.plann[i].read_x = 0;
		winCfg.plann[i].read_y = 0;
		winCfg.plann[i].plan_w = frameInfo.plane[i].w;
		winCfg.plann[i].plan_h = frameInfo.plane[i].h;
		winCfg.plann[i].w = get_plan_w(winCfg.plann[i].data_w,
                                winCfg.plann[i].showLineNum,
                                winCfg.plann[i].showTytle,
                                winCfg.plann[i].hideTop,
                                winCfg.plann[i].hideLeft);
		winCfg.plann[i].h = get_plan_h(winCfg.plann[i].data_h,
                                winCfg.plann[i].showLineNum,
                                winCfg.plann[i].showTytle,
                                winCfg.plann[i].hideTop,
                                winCfg.plann[i].hideLeft);
		winCfg.plann[i].x = cur_x + x_off_list[i];
		winCfg.plann[i].y = cur_y + y_off_list[i];
		//if (winCfg.plann[i].w + cur_x > winCfg.scr_cols) break;
		//if (winCfg.plann[i].h + cur_y > winCfg.scr_lines) break;
		RDWIN_DrawPlannBorder(&winCfg.plann[i]);
		//RDWIN_DrawPlannData(&winCfg.plann[i]);
		RDWIN_DrawLineNum(&winCfg.plann[i]);
		strncpy(winCfg.plann[i].tytle, tytle[i], PLAN_TYTLE_LENGTH);
		RDWIN_DrawTytle(&winCfg.plann[i]);
		cur_x += winCfg.plann[i].w;
		if (new_line[i])
		{
			cur_x = 0;
			cur_y = winCfg.plann[i].y + winCfg.plann[i].h;
		}
	}
	winCfg.status_bar_x = cur_x;
	winCfg.status_bar_y = cur_y;
    RDWIN_DrawProgressBar(&winCfg);
	
	refresh();
}

void RDWIN_Deinit(void)
{
    int i;
    for (i = 0; i < MAX_PLAN; i++)
    {
        if (NULL != winCfg.plann[i].data)
        {
            free(winCfg.plann[i].data);
        }
    }
	endwin();
	nl();
}

void RDWIN_Update(int x, int y)
{
    int i;
    int _x;
	int _y;
    /* crop */
    
    for (i = 0; i < MAX_PLAN; i++)
    {
        _x = frameInfo.plane[i].x_ds ? x / 2 : x;
		_y = frameInfo.plane[i].y_ds ? y / 2 : y;
        if (_x < winCfg.plann[i].read_x + winCfg.plann[i].data_w
            && _x >= winCfg.plann[i].read_x)
        {
            winCfg.plann[i].highlight_pos_x = _x - winCfg.plann[i].read_x;
        }
        else
        {
            if (_x >= winCfg.plann[i].read_x + winCfg.plann[i].data_w)
            {
				if (_x >= winCfg.plann[i].read_x + winCfg.plann[i].highlight_pos_x + winCfg.plann[i].data_w)
				{
					winCfg.plann[i].read_x = _x - winCfg.plann[i].highlight_pos_x;
					if (winCfg.plann[i].read_x > winCfg.plann[i].plan_w - winCfg.plann[i].data_w) {
						winCfg.plann[i].read_x = winCfg.plann[i].plan_w - winCfg.plann[i].data_w;
						winCfg.plann[i].highlight_pos_x = _x - winCfg.plann[i].read_x;
					}
				}
				else
				{
					winCfg.plann[i].highlight_pos_x = winCfg.plann[i].data_w - 1;
					winCfg.plann[i].read_x = _x - winCfg.plann[i].data_w + 1;
				}
            }
            else
            {
				if (_x <= winCfg.plann[i].read_x + winCfg.plann[i].highlight_pos_x - winCfg.plann[i].data_w)
				{
					winCfg.plann[i].read_x = _x - winCfg.plann[i].highlight_pos_x;
					if (winCfg.plann[i].read_x < 0) {
						winCfg.plann[i].read_x = 0;
						winCfg.plann[i].highlight_pos_x = _x;
					}
				}
				else
				{
					winCfg.plann[i].highlight_pos_x = 0;
					winCfg.plann[i].read_x = _x;
				}
            }
        }
        if (_y < winCfg.plann[i].read_y + winCfg.plann[i].data_h
            && _y >= winCfg.plann[i].read_y)
        {
            winCfg.plann[i].highlight_pos_y = _y - winCfg.plann[i].read_y;
        }
        else
        {
            if (_y >= winCfg.plann[i].read_y + winCfg.plann[i].data_h)
            {
				if (_y >= winCfg.plann[i].read_y + winCfg.plann[i].highlight_pos_y + winCfg.plann[i].data_h)
				{
					winCfg.plann[i].read_y = _y - winCfg.plann[i].highlight_pos_y;
					if (winCfg.plann[i].read_y > winCfg.plann[i].plan_h - winCfg.plann[i].data_h) {
						winCfg.plann[i].read_y = winCfg.plann[i].plan_h - winCfg.plann[i].data_h;
						winCfg.plann[i].highlight_pos_y = _y - winCfg.plann[i].read_y;
					}
				}
				else
				{
					winCfg.plann[i].highlight_pos_y = winCfg.plann[i].data_h - 1;
					winCfg.plann[i].read_y = _y - winCfg.plann[i].data_h + 1;
				}
            }
            else
            {
				if (_y <= winCfg.plann[i].read_y + winCfg.plann[i].highlight_pos_y - winCfg.plann[i].data_h)
				{
					winCfg.plann[i].read_y = _y - winCfg.plann[i].highlight_pos_y;
					if (winCfg.plann[i].read_y < 0) {
						winCfg.plann[i].read_y = 0;
						winCfg.plann[i].highlight_pos_y = _y;
					}
				}
				else
				{
					winCfg.plann[i].highlight_pos_y = 0;
					winCfg.plann[i].read_y = _y;
				}
            }
        }
        
        RDWIN_ReadPlannData(&winCfg.plann[i],
                        &frameInfo.plane[i],
                        winCfg.plann[i].read_x,
                        winCfg.plann[i].read_y);
        RDWIN_DrawPlannData(&winCfg.plann[i]);
        RDWIN_DrawLineNum(&winCfg.plann[i]);
    }
    RDWIN_DrawStatus(&winCfg);
    RDWIN_DrawProgressBar(&winCfg);
    refresh();
}

int get_delta_from_combo(int combo)
{
    combo /= 4;
    return combo < 4 ? 1 << combo : 1 << 4;
}

void show_help()
{
	printf("Usage: read_yuv [OPTION...] [FILE]\n");
	printf("Options:\n");
    printf("    -d           dec mode\n");
	printf("    -s size      set frame size (WxH)\n");
	printf("    -f format    set frame format (%s)\n", fmt_get_supported('|'));
    printf("    -o bmp file  dump bmp file\n");
    printf("    -p plann     dump single plann\n");
	printf("Examples:\n");
	printf("    read_yuv -s 1920x1080 -f 422p input.yuv\n");
}

int dump(void)
{
#define clip(x) ((x) > 255 ? 255 : (x) < 0 ? 0 : (x))

    /******************************************************
     * BT.601����
     ******************************************************/
    //R = clip(1.164 * (Y-16) + 1.596 * (V-128));
    //G = clip(1.164 * (Y-16) - 0.813 * (V-128) - 0.392 * (U-128));
    //B = clip(1.164 * (Y-16) + 2.017 * (U-128));
#define YUV_TO_RGB(Y, U, V, R, G, B) \
    { \
        R = clip((298 * (Y-16) + 409 * (V-128) + 128) >> 8); \
        G = clip((298 * (Y-16) - 100 * (U-128) - 208 * (V-128) + 128) >> 8); \
        B = clip((298 * (Y-16) + 516 * (U-128) + 128) >> 8); \
    }

    int bitlen = 24;
    FRAME_INFO *frame = &frameInfo;

    unsigned char lut[4];
    unsigned char hdr0[] = {0x42, 0x4d};
    struct {
        unsigned int bfSize;      ///< ���ֽڱ�ʾ�������ļ���С
        unsigned int bfReserved;  ///< ������һ������Ϊ0
        unsigned int bfOffBits;   ///< λͼ����ƫ��
        unsigned int biSize;      ///< λͼ��Ϣͷ�Ĵ�С��ͨ����28H
        unsigned int biWidth;     ///< λͼ���ؿ���
        unsigned int biHeight;    ///< λͼ���ظ߶�
        unsigned short biPlanes;    ///< λͼ��λ����������1
        unsigned short biBitCount;  ///< ����λ��
        unsigned int biCompression;   ///< ѹ��˵��
        unsigned int biSizeImage;     ///< λͼ���ݵĴ�С��4�ֽڵı���
        unsigned int biXPelsPerMeter; ///< ������/�ױ�ʾ��ˮƽ�ֱ��ʣ�Ĭ��0x0EC4
        unsigned int biYPelsPerMeter; ///< ������/�ױ�ʾ�Ĵ�ֱ�ֱ��ʣ�Ĭ��0x0EC4
        unsigned int biClrUsed;       ///< λͼʹ�õ���ɫ��
        unsigned int biClrImportant;  ///< ��Ҫ����ɫ��
    } hdr1;
    
    int line_size = 0;
    int data_size = 0;
    int lut_size = 0;
    int total_size = 0;
    unsigned char *pline = NULL;
    int i,j,x,y,p;
    unsigned char data[3];
    unsigned char tmp[3];
    int pid = 0;
    
    if (CIF_OUTPUT_SINGLE_PLAN)
    {
        pid = CIF_OUTPUT_PLAN;
        if (pid >= frame->plane_num || pid < 0)
        {
            printf("invalid plan id\n");
            return ERR_UNKNOWN_FORMAT;
        }
        bitlen = 8;
    }

    if (bitlen == 24)
    {
        line_size = (frame->w*3 + 3) & (~3);
        lut_size = 0;
    }
    else if (bitlen == 8)
    {
        line_size = (frame->w + 3) & (~3);
        lut_size = 256 * 4;
    }
    else
    {
        printf("wrong bitlen\n");
        return ERR_UNKNOWN_FORMAT;
    }
    data_size = line_size * frame->h;
    total_size = sizeof(hdr0) + sizeof(hdr1) + lut_size + data_size;

    hdr1.bfSize = total_size;
    hdr1.bfReserved = 0;
    hdr1.bfOffBits = sizeof(hdr0) + sizeof(hdr1) + lut_size;
    hdr1.biSize = 40;
    hdr1.biWidth = frame->w;
    hdr1.biHeight = frame->h;
    hdr1.biPlanes = 1;
    hdr1.biBitCount = bitlen;
    hdr1.biCompression = 0;
    hdr1.biSizeImage = data_size;
    hdr1.biXPelsPerMeter = 0x0EC4;
    hdr1.biYPelsPerMeter = 0x0EC4;
    hdr1.biClrUsed = 0;
    hdr1.biClrImportant = 0;

    
    pline = malloc(line_size);
    
    if (NULL == pline)
    {
        printf("malloc pline failed\n");
        return ERR_MEM_ALLOC_FAILED;
    }
    FILE *fout = fopen(CIF_OUTPUT_FILE_NAME, "wb");
    if (NULL == fout)
    {
        printf("open output file %s failed\n", CIF_OUTPUT_FILE_NAME);
        free(pline);
        return ERR_FILE_OPEN_FAILED;
    }

    fwrite(hdr0, 1, sizeof(hdr0), fout);
    fwrite(&hdr1, 1, sizeof(hdr1), fout);

    if (bitlen == 8)
    {
        /* lut */
        for (i = 0; i < 256; i++)
        {
            lut[0] = i;
            lut[1] = i;
            lut[2] = i;
            lut[3] = 0;
            fwrite(lut, 1, sizeof(lut), fout);
        }
        for (i = 0; i < frame->h; i++)
        {
            for (j = 0; j < frame->w; j++)
            {
                x = j / (frame->plane[pid].x_ds + 1);
                y = (frame->h-1-i) / (frame->plane[pid].y_ds + 1);
                pline[j] = read_pos(&frame->plane[pid], x, y);
            }
            fwrite(pline, 1, line_size, fout);
        }
    }
    else
    {
        if (CS_RGB == frame->cs || CS_YUV == frame->cs)
        {
            for (i = 0; i < frame->h; i++)
            {
                for (j = 0; j < frame->w; j++)
                {
                    for (p = 0; p < 3; p++)
                    {
                        x = j / (frame->plane[p].x_ds + 1);
                        y = (frame->h-1-i) / (frame->plane[p].y_ds + 1);
                        data[p] = read_pos(&frame->plane[p], x, y);
                    }
                    if (CS_RGB == frame->cs)
                    {
                        pline[j*3] = data[2];
                        pline[j*3+1] = data[1];
                        pline[j*3+2] = data[0];
                    }
                    else
                    {
                        YUV_TO_RGB(data[0], data[1], data[2],
                            tmp[0], tmp[1], tmp[2]);
                        pline[j*3] = tmp[2];
                        pline[j*3+1] = tmp[1];
                        pline[j*3+2] = tmp[0];
                    }
                }
                fwrite(pline, 1, line_size, fout);
            }
        }
        else
        {
            free(pline);
            fclose(fout);
            return ERR_UNKNOWN_FORMAT;
        }
    } 

    free(pline);
    fclose(fout);
    return 0;
}

int main(int argc, char *argv[])
{
	int x = 0, y = 0;
    int delta = 1;
    int clk_0 = 0, clk_1 = 0;
    int last_c = 0;
    int combo = 0;
	int i = 0;
	int err = 0;
	int result = 0;
	char *fname = NULL;
	FILE *pf = NULL;
	unsigned int file_size = 0;
	unsigned char *pbuf = NULL;
    int buf_size = 0;
	int old_lines = 0;
	int old_cols = 0;
	
	if (1 == argc || !strcmp(argv[1],"--help") || !strcmp(argv[1],"-h"))
	{
		show_help();
		return 0;
	}
	
	opterr = 0;
	while((result = getopt(argc, argv, "s:f:o:p:d")) != -1)
	{
		switch(result)
		{
            case 'd':
                CIF_DEC_MODE = 1;
                break;
			case 's':
				sscanf(optarg, "%dx%d", &CIF_WIDTH, &CIF_HEIGHT);
				break;
			case 'f':
                CIF_FORMAT = fmt_from_string(optarg);
				if (FMT_MAX == CIF_FORMAT)
				{
					err = ERR_UNKNOWN_FORMAT;
				}
				break;
            case 'o':
                CIF_OUTPUT = 1;
                strncpy(CIF_OUTPUT_FILE_NAME, optarg, sizeof(CIF_OUTPUT_FILE_NAME)-1);
                break;
            case 'p':
                CIF_OUTPUT_SINGLE_PLAN = 1;
                CIF_OUTPUT_PLAN = atoi(optarg);
                break;
			default:
				break;
		}
	}
	
	fname = argv[argc-1];
	if (err || 0 == CIF_WIDTH || 0 == CIF_HEIGHT || NULL == fname)
	{
		show_help();
		return 0;
	}

    buf_size = fmt_get_frame_size(CIF_FORMAT, CIF_WIDTH, CIF_HEIGHT);
    if (buf_size <= 0)
    {
        err = ERR_UNKNOWN_FORMAT;
        goto EXIT;
    }
	
	pf = fopen(fname, "rb");
	if (NULL == pf)
	{
		err = ERR_FILE_OPEN_FAILED;
		goto EXIT;
	}
	file_size = 0;
	fseek(pf, 0, SEEK_END);
	file_size = ftell(pf);
	if (file_size < buf_size)
	{
		err = ERR_FILE_SIZE_MISMATCH;
		goto EXIT;
	}
	fseek(pf, 0, SEEK_SET);
	
	pbuf = malloc(buf_size);
	if (NULL == pbuf)
	{
		err = ERR_MEM_ALLOC_FAILED;
		goto EXIT;
	}
	
	i = fread(pbuf, 1, buf_size, pf);
	if (i != buf_size)
	{
		err = ERR_FILE_READ_FAILED;
		goto EXIT;
	}

    i = fmt_init_frame_info(CIF_FORMAT, CIF_WIDTH, CIF_HEIGHT, pbuf, buf_size, &frameInfo);
    if (i < 0)
    {
        err = ERR_UNKNOWN_FORMAT;
        goto EXIT;
    }

    if (CIF_OUTPUT)
    {
        err = dump();
        //if (err)
        {
            goto EXIT;
        }
    }
    
	err = RDWIN_Init(MODE_1);
    if (err)
    {
        goto EXIT;
    }
	old_lines = LINES;
	old_cols = COLS;
	
    RDWIN_Update(x, y);
	while (1)
	{
		int c = getch();
        #if 0
        clk_1 = clock();
        if (last_c == c && clk_1 - clk_0 < 400)
        {
            combo ++;
        }
        else
        {
            combo = 0;
        }
        last_c = c;
        clk_0 = clk_1;
        delta = get_delta_from_combo(combo);
        #endif
        delta = 1;
		switch (c)
		{
			case 'q':
			case 'Q':
				goto EXIT;
				break;
			case 'w':
			case 'W':
                delta = winCfg.plann[0].data_h;
            case KEY_UP:
			    y = y - delta;
				break;
			case 's':
			case 'S':
                delta = winCfg.plann[0].data_h;
            case KEY_DOWN:
			    y = y + delta;
				break;
			case 'a':
			case 'A':
                delta = winCfg.plann[0].data_w;
            case KEY_LEFT:
			    x = x - delta;
				break;
			case 'd':
			case 'D':
                delta = winCfg.plann[0].data_w;
            case KEY_RIGHT:
			    x = x + delta;
				break;
            case 'x':
            case 'X':
                RDWIN_Deinit();
                scanf("%d", &x);
                err = RDWIN_Init(MODE_1);
				if (err) {
					goto EXIT;
				}
                break;
            case 'y':
            case 'Y':
                RDWIN_Deinit();
                scanf("%d", &y);
                err = RDWIN_Init(MODE_1);
				if (err) {
					goto EXIT;
				}
                break;
			default:
				break;
		}
		if (old_lines != LINES || old_cols != COLS)
		{
			RDWIN_Deinit();
			err = RDWIN_Init(MODE_1);
			if (err) {
				goto EXIT;
			}
			old_lines = LINES;
			old_cols = COLS;
		}
        if (x < 0) x = 0;
        if (x > CIF_WIDTH - 1) x = CIF_WIDTH - 1;
        if (y < 0) y = 0;
        if (y > CIF_HEIGHT - 1) y = CIF_HEIGHT - 1;
		RDWIN_Update(x, y);
	}
	
EXIT:
    if (err)
    {
        printf("Error: %d\n", err);
    }
	if (pbuf) {
		free(pbuf);
	}
	if (pf) {
		fclose(pf);
	}
	RDWIN_Deinit();
	return err;
}
