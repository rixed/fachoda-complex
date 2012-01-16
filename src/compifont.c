#include <math.h>
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
int SizeBigCharY, SizeBigCharX, SizeBigChar;
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
void main() {
	FILE *f;
	loadfont("font.tga", 16,7, 10);
	loadbigfont("bigfont.tga");
	f=fopen("font.comp","w+");
	fwrite(&font,112*10,sizeof(uchar),f);
	fclose(f);
	f=fopen("bigfont.comp","w+");
	fwrite(BigFont,SizeBigChar*13,sizeof(uchar),f);
	fclose(f);
	printf("SBCX=%d, SBCY=%d, SBC=%d\n",SizeBigCharX,SizeBigCharY,SizeBigChar);
}
