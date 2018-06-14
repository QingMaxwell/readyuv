#ifndef _FORMAT_H_
#define _FORMAT_H_

#define FMT_PLANE_MAX 4

#define FRAC(n, d) ((n)<<8 | (d))
#define MUL_FRAC(x, q) ((x) * (((q) & 0xff00) >> 8) / ((q) & 0xff))


typedef enum
{
    FMT_YUV444P = 0,
    FMT_YUV444P_FP16 = 1,
    FMT_YUV422P = 2,
    FMT_YUV422I = 3,
    FMT_YUV420P = 4,
    FMT_YUV420SP = 5,
    FMT_RGBI_U8 = 16,
    FMT_BGRI_U8 = 17,
    FMT_RGBI_FP16 = 18,
    FMT_BGRI_FP16 = 19,
    FMT_MAX = 0x7fffffff
} FMT_ID;

typedef enum
{
    BPP_X2 = FRAC(2,1),
    BPP_X3 = FRAC(3,1),
    BPP_X6 = FRAC(6,1),
    BPP_X3D2 = FRAC(3,2)
} FMT_BPP;

typedef enum
{
    PITCH_X1 = FRAC(1,1),
    PITCH_X2 = FRAC(2,1),
    PITCH_X3 = FRAC(3,1),
    PITCH_X6 = FRAC(6,1),
    PITCH_X1D2 = FRAC(1,2)
} FMT_PITCH;

typedef enum
{
    STAGE_X0 = FRAC(0,1),
    STAGE_X1 = FRAC(1,1),
    STAGE_X5D4 = FRAC(5,4),
    STAGE_X3D2 = FRAC(3,2),
    STAGE_X2 = FRAC(2,1),
    STAGE_X3 = FRAC(3,1),
    STAGE_X4 = FRAC(4,1),
} FMT_STAGE;

typedef enum
{
    CS_RGB = 0,
    CS_YUV = 1
} COLOR_SPACE;

typedef struct
{
    char *name;
    FMT_ID id;
} FMT_NAME;

typedef struct
{
    FMT_ID id;
    COLOR_SPACE cs;
    unsigned int plane_num;
    unsigned int bpp;
    unsigned int sample;
    unsigned int is_fp;
    FMT_STAGE stage[FMT_PLANE_MAX];
    unsigned int offset[FMT_PLANE_MAX];
    FMT_PITCH pitch[FMT_PLANE_MAX];
    unsigned int inter[FMT_PLANE_MAX];
    unsigned int x_ds[FMT_PLANE_MAX];
    unsigned int y_ds[FMT_PLANE_MAX];
} FMT_DESC;

typedef struct {
	unsigned char *buf;
	unsigned int w;
	unsigned int h;
	unsigned int pitch;
	unsigned int inter;
	int x_ds;
	int y_ds;
    unsigned int sample;
    unsigned int is_fp;
} FRAME_PLAN;

typedef struct {
    FMT_ID fmt;
    COLOR_SPACE cs;
    int w;
    int h;
    int size;
    int plane_num;
    FRAME_PLAN plane[FMT_PLANE_MAX];
} FRAME_INFO;

static FMT_DESC fmt_table[] = {
    {FMT_YUV444P, CS_YUV, 3, BPP_X3, 8, 0,
        {STAGE_X0, STAGE_X1, STAGE_X2},
        {0, 0, 0},
        {PITCH_X1, PITCH_X1, PITCH_X1},
        {1, 1, 1},
        {0, 0, 0},
        {0, 0, 0},
    },
    {FMT_YUV444P_FP16, CS_YUV, 3, BPP_X6, 16, 1,
        {STAGE_X0, STAGE_X2, STAGE_X4},
        {0, 0, 0},
        {PITCH_X2, PITCH_X2, PITCH_X2},
        {2, 2, 2},
        {0, 0, 0},
        {0, 0, 0},
    },
    {FMT_YUV422P, CS_YUV, 3, BPP_X2, 8, 0,
        {STAGE_X0, STAGE_X1, STAGE_X3D2},
        {0, 0, 0},
        {PITCH_X1, PITCH_X1D2, PITCH_X1D2},
        {1, 1, 1},
        {0, 1, 1},
        {0, 0, 0},
    },
    {FMT_YUV422I, CS_YUV, 3, BPP_X2, 8, 0,
        {STAGE_X0, STAGE_X0, STAGE_X0},
        {0, 1, 3},
        {PITCH_X2, PITCH_X2, PITCH_X2},
        {2, 4, 4},
        {0, 1, 1},
        {0, 0, 0},
    },
    {FMT_YUV420P, CS_YUV, 3, BPP_X3D2, 8, 0,
        {STAGE_X0, STAGE_X1, STAGE_X5D4},
        {0, 0, 0},
        {PITCH_X1, PITCH_X1D2, PITCH_X1D2},
        {1, 1, 1},
        {0, 1, 1},
        {0, 1, 1},
    },
    {FMT_YUV420SP, CS_YUV, 3, BPP_X3D2, 8, 0,
        {STAGE_X0, STAGE_X1, STAGE_X1},
        {0, 0, 1},
        {PITCH_X1, PITCH_X1, PITCH_X1},
        {1, 2, 2},
        {0, 1, 1},
        {0, 1, 1},
    },
    {FMT_RGBI_U8, CS_RGB, 3, BPP_X3, 8, 0,
        {STAGE_X0, STAGE_X0, STAGE_X0},
        {0, 1, 2},
        {PITCH_X3, PITCH_X3, PITCH_X3},
        {3, 3, 3},
        {0, 0, 0},
        {0, 0, 0},
    },
    {FMT_BGRI_U8, CS_RGB, 3, BPP_X3, 8, 0,
        {STAGE_X0, STAGE_X0, STAGE_X0},
        {2, 1, 0},
        {PITCH_X3, PITCH_X3, PITCH_X3},
        {3, 3, 3},
        {0, 0, 0},
        {0, 0, 0},
    },
    {FMT_RGBI_FP16, CS_RGB, 3, BPP_X6, 16, 1,
        {STAGE_X0, STAGE_X0, STAGE_X0},
        {0, 2, 4},
        {PITCH_X6, PITCH_X6, PITCH_X6},
        {6, 6, 6},
        {0, 0, 0},
        {0, 0, 0},
    },
    {FMT_BGRI_FP16, CS_RGB, 3, BPP_X6, 16, 1,
        {STAGE_X0, STAGE_X0, STAGE_X0},
        {4, 2, 0},
        {PITCH_X6, PITCH_X6, PITCH_X6},
        {6, 6, 6},
        {0, 0, 0},
        {0, 0, 0},
    },
    {FMT_MAX}
};

static FMT_NAME name_table[] = {
    {"444p", FMT_YUV444P},
    {"444pfp16", FMT_YUV444P_FP16},
    {"422p", FMT_YUV422P},
    {"422i", FMT_YUV422I},
    {"420p", FMT_YUV420P},
    {"420sp", FMT_YUV420SP},
    {"rgbiu8", FMT_RGBI_U8},
    {"bgriu8", FMT_BGRI_U8},
    {"rgbifp16", FMT_RGBI_FP16},
    {"bgrifp16", FMT_BGRI_FP16},
    {NULL}
};


//#####################################################################################################

#define MOVIDIUS_FP32
#define F32_EX_INEXACT     0x00000001//0x00000020
#define F32_EX_DIV_BY_ZERO 0x00000002//0x00000004
#define F32_EX_INVALID     0x00000004//0x00000001
#define F32_EX_UNDERFLOW   0x00000008//0x00000010
#define F32_EX_OVERFLOW    0x00000010//0x00000008

#define EXTRACT_F16_SIGN(x)   ((x >> 15) & 0x1)
#define EXTRACT_F16_EXP(x)    ((x >> 10) & 0x1F)
#define EXTRACT_F16_FRAC(x)   (x & 0x000003FF)

#define F16_IS_SNAN(x)      (((x & 0x7E00) == 0x7C00)&&((x & 0x1FF)> 0))
#define PACK_F32(x, y, z)     ((x << 31) + (y << 23) + z)


typedef union
{
    unsigned int u32;
    float f32;
}u32f32;

unsigned int exceptionsReg;
unsigned int* exceptions = &exceptionsReg;

unsigned int f16_shift_left(unsigned int op, unsigned int cnt)
{
    unsigned int result;
    if (cnt == 0)
    {
        result = op;
    }
    else if (cnt < 32)
    {
        result = (op << cnt);
    }
    else 
    {
        result = 0;
    }
    return result;
}

float f16Tof32(unsigned int x)
{
    unsigned int sign;
    int exp;
    unsigned int frac;
    unsigned int result;
    u32f32       u;

    frac = EXTRACT_F16_FRAC(x);
    exp  = EXTRACT_F16_EXP(x);
    sign = EXTRACT_F16_SIGN(x);
    if (exp == 0x1F) 
    {
        if (frac != 0)
        {
            // NaN
            if (F16_IS_SNAN(x))
            {
                *exceptions |= F32_EX_INVALID;
            }
            result = 0;
            //Get rid of exponent and sign
#ifndef MOVIDIUS_FP32
            result = x << 22;
            result = f32_shift_right(result, 9);
            result |= ((sign << 31) | 0x7F800000);
#else
            result |= ((sign << 31) | 0x7FC00000);
#endif
        }
        else
        {
            //infinity
            result = PACK_F32(sign, 0xFF, 0);
        }
    }
    else if (exp == 0)
    {
        //either denormal or zero
        if (frac == 0)
        {
            //zero
            result = PACK_F32(sign, 0, 0);
        }
        else
        {
            //subnormal
#ifndef MOVIDIUS_FP32
            f16_normalize_subnormal(&frac, &exp);
            exp--;
            // ALDo: is the value 13 ok??
            result = f16_shift_left(frac, 13);
            // exp = exp + 127 - 15 = exp + 112
            result = PACK_F32(sign, (exp + 0x70), result);
#else
            result = PACK_F32(sign, 0, 0);
#endif
        }
    }
    else
    {
        // ALDo: is the value 13 ok??
        result = f16_shift_left(frac, 13);
        result = PACK_F32(sign, (exp + 0x70), result);
    }
    
    u.u32 = result;
    return u.f32; //andreil
}


unsigned int read_pos(FRAME_PLAN *frame, int x, int y)
{
	unsigned char *buf = frame->buf;
	int w = frame->w;
	int h = frame->h;
	int pitch = frame->pitch;
	int inter = frame->inter;
    unsigned int sample = frame->sample;
    unsigned int isFp = frame->is_fp;
    
    if (sample == 8)
    {
    	if (x < w && y < h)
    		return buf[pitch*y+x*inter];
        else
    	    return 0xff;
    }
    else if (sample == 16)
    {
        if (x < w && y < h) {
            unsigned short *_p = (unsigned short *)(&buf[pitch*y+x*inter]);
            if (isFp)
                return (unsigned int)f16Tof32(*_p);
            else
                return *_p;
        }
        else
            return 0xffff;
    }
    else if (sample == 32)
    {
        if (x < w && y < h)
            return *((unsigned int *)(&buf[pitch*y+x*inter]));
        else
            return 0xffffffff;
    }
    return 0xffffffff;
}

unsigned short read_pos_16bit(FRAME_PLAN *frame, int x, int y)
{
	unsigned char *buf = frame->buf;
    unsigned short *buf16;
	int w = frame->w;
	int h = frame->h;
	int pitch = frame->pitch;
	int inter = frame->inter;
	if (x < w && y < h) {
        buf16 = (unsigned short *)&buf[pitch*y+x*inter];
		return *buf16;
	}
	return 0xffff;
}

char *fmt_get_supported(char c)
{
    static char *listbuf = NULL;
    FMT_NAME *entry = NULL;
    char *p;
    if (listbuf == NULL)
    {
        int size_required = 0;
        
        for (entry = &name_table[0]; entry->name != NULL; entry++)
        {
            size_required += strlen(entry->name) + 1;
        }
        if (size_required > 0)
        {
            listbuf = malloc(size_required);
        }
        if (NULL == listbuf)
        {
            listbuf = (char *)(-1);
        }
    }
    if (listbuf == (char *)(-1))
    {
        return "EEEE";
    }
    p = listbuf;
    for (entry = &name_table[0]; entry->name != NULL; entry++)
    {
        strcpy(p,entry->name);
        p += strlen(entry->name);
        *p++ = c;
    }
    if (p > listbuf)
    {
        *(--p) = '\0';
    }
    return listbuf;
}

FMT_ID fmt_from_string(char *name)
{
    FMT_NAME *entry;
    int find = 0;
    for (entry = &name_table[0]; entry->name != NULL; entry++)
    {
        if (!strcmp(name, entry->name))
        {
            find = 1;
            break;
        }
    }
    if (!find)
    {
        return FMT_MAX;
    }
    return entry->id;
}

int fmt_get_frame_size(FMT_ID fmt,
                    unsigned int w,
                    unsigned int h)
{
    FMT_DESC *desc;
    int find = 0;
    unsigned int frame_size = 0;
    
    for (desc = &fmt_table[0]; desc->id != FMT_MAX; desc++)
    {
        if (desc->id == fmt)
        {
            find = 1;
            break;
        }
    }
    if (!find)
    {
        return -1;
    }

    frame_size = MUL_FRAC(w * h, desc->bpp);
    return frame_size;
}

int fmt_init_frame_info(FMT_ID fmt,
                    unsigned int w,
                    unsigned int h,
                    unsigned char *buf,
                    unsigned int buf_size,
                    FRAME_INFO *info)
{
    int i = 0;
    int tmp = 0;
    FMT_DESC *desc;
    int find = 0;
    unsigned int frame_size = 0;
    FRAME_PLAN *plane = NULL;
    
    for (desc = &fmt_table[0]; desc->id != FMT_MAX; desc++)
    {
        if (desc->id == fmt)
        {
            find = 1;
            break;
        }
    }
    if (!find)
    {
        return -1;
    }

    frame_size = MUL_FRAC(w * h, desc->bpp);
    if (buf_size < frame_size)
    {
        return -2;
    }

    info->fmt = fmt;
    info->cs = desc->cs;
    info->w = w;
    info->h = h;
    info->size = frame_size;
    info->plane_num = desc->plane_num;

    for (i = 0; i < desc->plane_num; i++)
    {
        plane = info->plane + i;
        plane->buf = buf + MUL_FRAC(w * h, desc->stage[i]) + desc->offset[i];
        plane->w = w / (desc->x_ds[i] + 1);
        plane->h = h / (desc->y_ds[i] + 1);
        plane->pitch = MUL_FRAC(w, desc->pitch[i]);
        plane->inter = desc->inter[i];
        plane->x_ds = desc->x_ds[i];
        plane->y_ds = desc->y_ds[i];
        plane->sample = desc->sample;
        plane->is_fp = desc->is_fp;
    }
    return 0;
}

#endif

