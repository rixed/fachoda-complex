// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "proto.h"
unsigned char num[10][8]= {
    { 0x3E,0x41,0x41,0x41,0x41,0x41,0x41,0x3E },        // 0
    { 0x08,0x18,0x38,0x18,0x18,0x18,0x18,0x3C },        // 1
    { 0x3E,0x63,0x43,0x06,0x0C,0x18,0x39,0x7F },        // 2
    { 0x3E,0x63,0x43,0x07,0x03,0x43,0x63,0x3E },        // 3
    { 0x30,0x30,0x30,0x36,0x3F,0x06,0x06,0x0F },
    { 0x7F,0x61,0x60,0x7E,0x63,0x03,0x47,0x3E },        // 5
    { 0x3E,0x63,0x60,0x6C,0x73,0x63,0x36,0x1C },
    { 0x7F,0x43,0x03,0x0E,0x38,0x70,0x60,0x60 },        // 7
    { 0x3E,0x63,0x63,0x3E,0x63,0x63,0x63,0x3E },
    { 0x3E,0x63,0x63,0x33,0x1F,0x03,0x06,0x0C }     // 9
};
void pnumchar(int n, int x, int y, int c) {
    int i, l;
    for (l=0; l<8; l++, y++) {
        for (i=128; i>=1; i>>=1, x++) if (num[n][l]&i) ((int*)videobuffer)[x+SX*y]=c;
        x-=8;
    }
}
void pnum(int n, int x, int y, int c, char just) {
    char sig=2*(n>=0)-1;
    int m=n;
    if (just==1) { // justifié à gauche
        if (sig==1) x-=8;
        while (m!=0) { x+=8; m/=10; };
    }
    do {
        pnumchar(sig*(n%10),x,y,c);
        x-=8;
        n/=10;
    } while (n!=0);
    if (sig==-1) for (m=x+2; m<x+6; m++) ((int*)videobuffer)[m+SX*(y+4)]=c;
}
void pnumchara(int n, int x, int y, int c) {    // a = dans le mapping
    int i, l;
    for (l=0; l<8; l++, y++) {
        for (i=128; i>=1; i>>=1, x++) if (num[n][l]&i) ((int*)mapping)[x+(y<<8)]=c;
        x-=8;
    }
}
void pnuma(int n, int x, int y, int c, char just) {
    char sig=2*(n>=0)-1;
    int m=n;
    if (just==1) { // justifié à gauche
        if (sig==1) x-=8;
        while (m!=0) { x+=8; m/=10; };
    }
    while (n!=0) {
        pnumchara(sig*(n%10),x,y,c);
        x-=8;
        n/=10;
    };
    if (sig==-1) for (m=x+2; m<x+6; m++) ((int*)mapping)[m+((y+4)<<8)]=c;
}
uchar font[112][10];
int SizeCharY=10;

void loadfont(char *fn, int nx, int ny, int cy) {
    FILE *fil;
    int x,y,fx,fy,i,sx=0,sy=0;
    pixel *itmp;
    SizeCharY=cy;
    if ((fil=fopen(fn,"r"))==NULL) {
        perror("fopen font file"); exit(-1);
    }
    fseek(fil,12,SEEK_SET);
    fread(&sx,2,1,fil);
    fread(&sy,2,1,fil);
    fseek(fil,-sx*sy*sizeof(pixel),SEEK_END);
    itmp=(pixel*)malloc(sx*sy*sizeof(pixel));
    fread(itmp,sizeof(pixel),sx*sy,fil);
    fclose(fil);
    for (fy=0; fy<ny; fy++)
        for (fx=0; fx<nx; fx++) {
            for (y=0; y<SizeCharY; y++) {
                for (i=0, x=0; x<8; x++) {
                    i<<=1;
                    i+=itmp[(fy*cy+y)*sx+8*fx+x].g!=0;
                }
                font[fy*nx+fx][y]=i;
            }
        }
    free(itmp);
}
uchar *BigFont;
int SizeBigCharY=50, SizeBigCharX=50, SizeBigChar=2500;

void loadbigfont(char *fn) {
    FILE *fil;
    int x,y,fx,sx=0,sy=0;
    pixel32 *itmp;
    if ((fil=fopen(fn,"r"))==NULL) {
        perror("fopen bigfont file"); exit(-1);
    }
    fseek(fil,12,SEEK_SET);
    fread(&sx,2,1,fil);
    fread(&sy,2,1,fil);
    SizeBigCharY=sy;
    SizeBigCharX=sx/13;
    SizeBigChar=SizeBigCharX*SizeBigCharY;
    fseek(fil,-sx*sy*sizeof(pixel32),SEEK_END);
    itmp=(pixel32*)malloc(sx*sy*sizeof(pixel32));
    fread(itmp,sizeof(pixel32),sx*sy,fil);
    fclose(fil);
    BigFont=(uchar*)malloc(sx*sy*sizeof(uchar));
    for (fx=0; fx<13; fx++) {
        for (y=0; y<SizeBigCharY; y++) {
            for (x=0; x<SizeBigCharX; x++)
                BigFont[fx*SizeBigChar+y*SizeBigCharX+x]=itmp[fx*SizeBigCharX+y*sx+x].u;
        }
    }
    free(itmp);
}

static int addsat_byte(int b1, int b2, int shft) {
    int const mask = 0xff<<shft;
    int const a = (b1 & mask) + (b2 & mask);
    return a > mask ? mask : a;
}
static int subsat_byte(int b1, int b2, int shft) {
    int const mask = 0xff<<shft;
    int const a = (b1 & mask) - (b2 & mask);
    return a > mask /* wrap */ || a < (1<<shft) ? 0 : a;
}
void MMXAddSatC(int *dst, int coul) {
    int v = *dst;
    *dst = (
        addsat_byte(v, coul, 0) |
        addsat_byte(v, coul, 8) |
        addsat_byte(v, coul, 16)
    );
}
static void MMXSubSatC(int *dst, int coul) {
    int v = *dst;
    *dst = (
        subsat_byte(v, coul, 0) |
        subsat_byte(v, coul, 8) |
        subsat_byte(v, coul, 16)
    );
}
void MMXAddSat(int *dst, int byte) {
    int b = byte & 0xff;
    MMXAddSatC(dst, b | (b<<8) | (b<<16));
}
void MMXSubSat(int *dst, int byte) {
    int b = byte & 0xff;
    MMXSubSatC(dst, b | (b<<8) | (b<<16));
}
void MMXAddSatInt(int *dst, int byte, int n)
{
    while (n--) MMXAddSat(dst++, byte);
}

void pbignumchar(int n, int x, int y, int coul) {
    int xx, yy;
    for (yy=0; yy<SizeBigCharY; yy++) {
        for (xx=0; xx<SizeBigCharX; xx++) {
            if (BigFont[n*SizeBigChar+yy*SizeBigCharX+xx]) {
                if (x+xx>=0 && x+xx<SX && y+yy>=0 && y+yy<SY)
                    MMXAddSatC((int*)videobuffer+x+xx+SX*(y+yy),coul);
            }
        }
    }
}
void pbignum(int n, int x, int y, char just, char tot, char dolsig) {
    int m, c;
    uchar sig=0;
    if (n<0) { sig=1; n=-n; }
    if (tot) c=0x7F7F7F;
    else {
        if (sig) c=0xA02020;
        else c=0x2020A0;
    }
    m=n;
    if (just==1) { // justifié à gauche
        while (m!=0) { x+=SizeBigCharX; m/=10; };
    } else if (just==2) {
        x-=SizeBigCharX>>1;
        while (m!=0) { x+=SizeBigCharX>>1; m/=10; };
    } else x-=SizeBigCharX;
    while (n!=0) {
        pbignumchar(n%10,x,y,c);
        x-=SizeBigCharX;
        n/=10;
    }
    if (dolsig) pbignumchar(12,x,y,c);
    if (!tot || sig) pbignumchar(10+sig,x-SizeBigCharX,y,c);
}

static int is_printable(int m) {
    return m>=16 && m<=112+16;
}

int TextClipX1=0, TextClipX2=0, TextColfont=0;
void pcharady(int m, int *v, int c, int off) {
    int i, l, y, x;
    assert(is_printable(m));
    for (l=0, y=0; l<10; l++, y+=off) {
        for (x=0, i=128; i>=1; i>>=1, x++) if (font[m-16][l]&i) v[x+y]=c;
        x-=8;
    }
}
void pcharlent(int m, int x, int y, int c) {
    int i, l;
    assert(is_printable(m));
    for (l=0; l<10; l++, y++) {
        for (i=128; i>=1; i>>=1, x++) if (font[m-16][l]&i) {
            if (x>=0 && x<SX-1 && y>=0 && y<SY-1) {
                ((int*)videobuffer)[x+SX*y]=c;
                ((int*)videobuffer)[x+SX*y+SX+1]=0;
            }
        }
        x-=8;
    }
}
void pchar(int m, int x, int y, int c) {
    int i, l;
    assert(is_printable(m));
    for (l=0; l<10; l++, y++) {
        for (i=128; i>=1; i>>=1, x++) if (font[m-16][l]&i) {
            ((int*)videobuffer)[x+SX*y]=c;
            ((int*)videobuffer)[x+SX*y+SX+1]=TextColfont;
        }
        x-=8;
    }
}
void pword(char *m, int x, int y, int c) {
    do {
        pchar(*m,x,y,c);    // pchar normalement, mais sinon present bug qd les bouttons clippent
        x+=6;
        m++;
    } while (*m!=' ' && is_printable(*m));
}

void pwordlent(char *m, int x, int y, int c) {
    do {
        pcharlent(*m,x,y,c);
        x+=6;
        m++;
    } while (*m!=' ' && is_printable(*m));
}

void pstr(char *m, int y, int c) {
    int l,ll,x;
    int sx1=TextClipX1?TextClipX1:0;
    int sx2=TextClipX2?TextClipX2:SX;
    if ((l=strlen(m)*6)<(sx2-sx1)) x=sx1+((sx2-sx1-l)>>1);
    else {
        l=strlen(m);
        x=sx1;
        do {
            do l--; while(l && m[l]!=' ' && is_printable(m[l]));
            if (l && l*6<sx2) {
                x=sx1+((sx2-sx1-l*6)>>1);
                break;
            }
        } while (l);
    }
    do {
        l=0;
        do l++; while(m[l]!=' ' && is_printable(m[l]));
        if (x+l*6<sx2) {
            pword(m,x,y,c);
        } else {
            y+=SizeCharY+1;
            if ((ll=strlen(m)*6)<(sx2-sx1)) x=sx1+((sx2-sx1-ll)>>1);
            else {
                x=sx1;
                ll=strlen(m);
                do {
                    do ll--; while(ll && m[ll]!=' ' && is_printable(m[ll]));
                    if (ll && ll*6<sx2) {
                        x=sx1+((sx2-sx1-ll*6)>>1);
                        break;
                    }
                } while (ll);
            }
            pword(m,x,y,c);
        }
        x+=l*6+5;
        m+=l;
        while(*m!='\0' && (*m==' ' || !is_printable(*m))) m++;
    } while (*m!='\0');
}

void pstrlent(char *m, int y, int c) {
    int l,ll,x;
    int sx1=TextClipX1?TextClipX1:0;
    int sx2=TextClipX2?TextClipX2:SX;
    if ((l=strlen(m)*6)<(sx2-sx1)) x=sx1+((sx2-sx1-l)>>1);
    else {
        l=strlen(m);
        x=sx1;
        do {
            do l--; while(l && m[l]!=' ' && is_printable(m[l]));
            if (l && l*6<sx2) {
                x=sx1+((sx2-sx1-l*6)>>1);
                break;
            }
        } while (l);
    }
    do {
        l=0;
        do l++; while(m[l]!=' ' && is_printable(m[l]));
        if (x+l*6<sx2) {
            pwordlent(m,x,y,c);
        } else {
            y+=SizeCharY+1;
            if ((ll=strlen(m)*6)<(sx2-sx1)) x=sx1+((sx2-sx1-ll)>>1);
            else {
                x=sx1;
                ll=strlen(m);
                do {
                    do ll--; while(ll && m[ll]!=' ' && is_printable(m[ll]));
                    if (ll && ll*6<sx2) {
                        x=sx1+((sx2-sx1-ll*6)>>1);
                        break;
                    }
                } while (ll);
            }
            pwordlent(m,x,y,c);
        }
        x+=l*6+5;
        m+=l;
        while(*m!='\0' && (*m==' ' || !is_printable(*m))) m++;
    } while (*m!='\0');
}

