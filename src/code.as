[SEGMENT .text]
[EXTERN main]
[EXTERN polygouro]
[GLOBAL kread]
kread:
	movzx eax,byte [esp+4]
	bt [keytab],eax
	setc al
	retn
[GLOBAL kreset]
kreset:
	movzx eax,byte [esp+4]
	btr [keytab],eax
	setc al
	retn

[GLOBAL nuplot]
nuplot:
	pushad
	mov edi,[esp+32+8]	; y
	mov ecx,[esp+32+12]	; ecx=yoff (=r)
	cmp edi,[SY]
	jae near .return
	cmp ecx,0
	jle near .return
	imul edi,[SX]
	shl edi,2
	add edi,[videobuffer]	; edi = videobuffer+y*SX
	mov ebx,[esp+32+4]	; ebx=x
	xor edx,edx				; edx=xoff
	mov ebp,ecx
	mov byte [newyoff],1
	neg ebp					; ebp=balance (=-r)
	mov eax,ecx
	mov esi,[zfac]
	mov dword [ix],0
	imul eax,esi
	mov dword [iy],eax
.do:
	mov eax,[iy]
	mov al,ah
	mov [val],eax
	mov [val+2],al
	add ebx,edx
	cmp ebx,[SX]
	jae .fi1
	movd mm0,[edi+ebx*4]
	paddusb mm0,[val]
	movd [edi+ebx*4],mm0
.fi1:
	sub ebx,edx	; ebx=x
	or edx,edx
	jz .fi2
	sub ebx,edx
	cmp ebx,[SX]
	jae .fi21
	movd mm0,[edi+ebx*4]
	paddusb mm0,[val]
	movd [edi+ebx*4],mm0
.fi21:
	add ebx,edx
.fi2:
	test byte [newyoff],1
	jz .fi3
	cmp ecx,edx
	je .fi3
		mov eax,[ix]
		mov al,ah
		mov [val2],eax
		mov [val2+2],al	
		add ebx,ecx
		cmp ebx,[SX]
		jae .fi4
		movd mm0,[edi+ebx*4]
		paddusb mm0,[val2]
		movd [edi+ebx*4],mm0
	.fi4:
		sub ebx,ecx
		sub ebx,ecx
		cmp ebx,[SX]
		jae .fi31
		movd mm0,[edi+ebx*4]
		paddusb mm0,[val2]
		movd [edi+ebx*4],mm0
	.fi31:
		add ebx,ecx
.fi3:
	add ebp,edx
	mov byte [newyoff],1
	add ebp,edx
	inc edx
	cmp ebp,0
	jl .fi5
	dec ecx
	sub ebp,ecx
	mov byte [newyoff],2
	sub ebp,ecx
	sub dword [iy],esi
.fi5:
	dec byte	[newyoff]
	add dword [ix],esi
	cmp edx,ecx
	jle near .do
.return:
	popad
	retn
	
[GLOBAL fuplot]
fuplot:
	pushad
	mov edi,[esp+32+8]	; y
	mov ecx,[esp+32+12]	; ecx=yoff (=r)
	cmp edi,[SY]
	jae near .return
	cmp ecx,0
	jle near .return
	imul edi,[SX]
	shl edi,2
	add edi,[videobuffer]	; edi = videobuffer+y*SX
	mov ebx,[esp+32+4]	; ebx=x
	xor edx,edx				; edx=xoff
	mov ebp,ecx
	mov byte [newyoff],1
	neg ebp					; ebp=balance (=-r)
	mov eax,ecx
	mov esi,[zfac]
	mov dword [ix],0
	imul eax,esi
	mov dword [iy],eax
.do:
	mov eax,[iy]
	mov al,ah
	mov [val],eax
	mov [val+2],al
	add ebx,edx
	cmp ebx,[SX]
	jae .fi1
	movd mm0,[edi+ebx*4]
	psubusb mm0,[val]
	movd [edi+ebx*4],mm0
.fi1:
	sub ebx,edx	; ebx=x
	or edx,edx
	jz .fi2
	sub ebx,edx
	cmp ebx,[SX]
	jae .fi21
	movd mm0,[edi+ebx*4]
	psubusb mm0,[val]
	movd [edi+ebx*4],mm0
.fi21:
	add ebx,edx
.fi2:
	test byte [newyoff],1
	jz .fi3
	cmp ecx,edx
	je .fi3
		mov eax,[ix]
		mov al,ah
		mov [val2],eax
		mov [val2+2],al	
		add ebx,ecx
		cmp ebx,[SX]
		jae .fi4
		movd mm0,[edi+ebx*4]
		psubusb mm0,[val2]
		movd [edi+ebx*4],mm0
	.fi4:
		sub ebx,ecx
		sub ebx,ecx
		cmp ebx,[SX]
		jae .fi31
		movd mm0,[edi+ebx*4]
		psubusb mm0,[val2]
		movd [edi+ebx*4],mm0
	.fi31:
		add ebx,ecx
.fi3:
	add ebp,edx
	mov byte [newyoff],1
	add ebp,edx
	inc edx
	cmp ebp,0
	jl .fi5
	dec ecx
	sub ebp,ecx
	mov byte [newyoff],2
	sub ebp,ecx
	sub dword [iy],esi
.fi5:
	dec byte	[newyoff]
	add dword [ix],esi
	cmp edx,ecx
	jle near .do
.return:
	popad
	retn

[GLOBAL phplot]
phplot:
	pushad
	mov edi,[esp+32+8]	; y
	mov ecx,[esp+32+12]	; ecx=yoff (=r)
	cmp edi,[SY]
	jae near .return
	cmp ecx,0
	jle near .return
	imul edi,[SX]
	shl edi,2
	add edi,[videobuffer]	; edi = videobuffer+y*SX
	mov ebx,[esp+32+4]	; ebx=x
	xor edx,edx				; edx=xoff
	mov ebp,ecx
	mov byte [newyoff],1
	neg ebp					; ebp=balance (=-r)
	mov eax,ecx
	mov esi,[zfac]
	mov dword [ix],0
	imul eax,esi
	mov dword [iy],eax
.do:
	mov eax,[iy]
	mov al,ah
	shl eax,8
	mov [val],eax
;	mov [val+2],al
	add ebx,edx
	cmp ebx,[SX]
	jae .fi1
	movd mm0,[edi+ebx*4]
	paddusb mm0,[val]
	movd [edi+ebx*4],mm0
.fi1:
	sub ebx,edx	; ebx=x
	or edx,edx
	jz .fi2
	sub ebx,edx
	cmp ebx,[SX]
	jae .fi21
	movd mm0,[edi+ebx*4]
	paddusb mm0,[val]
	movd [edi+ebx*4],mm0
.fi21:
	add ebx,edx
.fi2:
	test byte [newyoff],1
	jz .fi3
	cmp ecx,edx
	je .fi3
		mov eax,[ix]
		mov al,ah
		shl eax,8
		mov [val2],eax
;		mov [val2+2],al	
		add ebx,ecx
		cmp ebx,[SX]
		jae .fi4
		movd mm0,[edi+ebx*4]
		paddusb mm0,[val2]
		movd [edi+ebx*4],mm0
	.fi4:
		sub ebx,ecx
		sub ebx,ecx
		cmp ebx,[SX]
		jae .fi31
		movd mm0,[edi+ebx*4]
		paddusb mm0,[val2]
		movd [edi+ebx*4],mm0
	.fi31:
		add ebx,ecx
.fi3:
	add ebp,edx
	mov byte [newyoff],1
	add ebp,edx
	inc edx
	cmp ebp,0
	jl .fi5
	dec ecx
	sub ebp,ecx
	mov byte [newyoff],2
	sub ebp,ecx
	sub dword [iy],esi
.fi5:
	dec byte	[newyoff]
	add dword [ix],esi
	cmp edx,ecx
	jle near .do
.return:
	popad
	retn

	
[GLOBAL calcasm]
calcasm:
	push edx
	mov eax,[esp+4+4+4]
	imul dword [esp+4+4+8]
	shrd eax,edx,13
	add eax,[esp+4+4]
	pop edx
	retn
	
[GLOBAL MMXDarkenLine]
; void MMXDarkenLine(pixel *vid,int dx,int c,pixel *vidt);
MMXDarkenLine:
	pushad
	mov edi,[esp+32+4]	; vid
	mov ecx,[esp+32+8]	; dx
	mov eax,[esp+32+12]	; c
	mov esi,[esp+32+16]	; vidt
	mov [val],eax
	mov [val+3],eax
	mov [val+6],ax
	movq mm0,[val]
	ror dword [val],16
	ror dword [val+4],16
	movq mm1,[val]
	ror dword [val],16
	ror dword [val+4],16
	movq mm2,[val]
	shr ecx,3
	test byte [esp+32+8],11b
	adc ecx,0
.x:
	movq mm3,[esi]
	movq mm4,[esi+8]
	movq mm5,[esi+16]
	psubusb mm3,mm0
	psubusb mm4,mm1
	psubusb mm5,mm2
	movq [esi],mm3
	movq [esi+8],mm4
	movq [esi+16],mm5
	movq [edi],mm3
	add esi,24
	movq [edi+8],mm4
	movq [edi+16],mm5
	add edi,24
	dec ecx
	jnz .x
	popad
	retn

[GLOBAL poly]
;void poly (veci *p1, veci *p2, veci *p3) {
;	vect2d l1,l2,l3;
;	l1.x=_DX+(p1->x*_DX)/p1->z;
;	l1.y=_DY+(p1->y*_DX)/p1->z;
;	l2.x=_DX+(p2->x*_DX)/p2->z;
;	l2.y=_DY+(p2->y*_DX)/p2->z;
;	l3.x=_DX+(p3->x*_DX)/p3->z;
;	l3.y=_DY+(p3->y*_DX)/p3->z;
;	polyflat(&l1,&l2,&l3,&coulpoly);
;}
poly:
	pushad
	mov edi,[esp+32+4]
	mov eax,[edi]				; p1x
	imul dword [_DX]
	idiv dword [edi+8]		; p1z
	add eax,[_DX]
	mov [l1],eax
	mov eax,[edi+4]			; p1y
	imul dword [_DX]
	idiv dword [edi+8]		; p1z
	add eax,[_DY]
	dec eax	; petite gruge pour que la route n'apparaisse pas à l'horizon au dessus du sol...
	mov [l1+4],eax
	mov eax,[edi+12]			; coul
	mov [l1+8],eax
	mov edi,[esp+32+4+4]
	mov eax,[edi]				; p2x
	imul dword [_DX]
	idiv dword [edi+8]		; p2z
	add eax,[_DX]
	mov [l2],eax
	mov eax,[edi+4]			; p2y
	imul dword [_DX]
	idiv dword [edi+8]		; p2z
	add eax,[_DY]
	dec eax	; petite gruge pour que la route n'apparaisse pas à l'horizon au dessus du sol...
	mov [l2+4],eax
	mov eax,[edi+12]			; coul
	mov [l2+8],eax
	mov edi,[esp+32+4+8]
	mov eax,[edi]				; p3x
	imul dword [_DX]
	idiv dword [edi+8]		; p3z
	add eax,[_DX]
	mov [l3],eax
	mov eax,[edi+4]			; p3y
	imul dword [_DX]
	idiv dword [edi+8]		; p3z
	add eax,[_DY]
	dec eax	; petite gruge pour que la route n'apparaisse pas à l'horizon au dessus du sol...
	mov [l3+4],eax
	mov eax,[edi+12]			; coul
	mov [l3+8],eax
	; si sens direct, afficher
	mov eax,[l2]	; eax=p2.x
	mov ebx,[l3+4]	; ebx=p3.y
	sub eax,[l1]	; eax=l2.x-l1.x
	sub ebx,[l1+4]	; ebx=p3.y-p1.y
	imul eax,ebx	; eax = K1
	mov ecx,[l2+4]	; ecx=p2.y
	mov ebx,[l3]	; ebx=p3.x
	sub ecx,[l1+4]	; ecx=p2.y-p1.y
	sub ebx,[l1]	; ebx=p3.x-p1.x
	imul ecx,ebx	; ecx=K2
	sub eax,ecx		; eax=vectZ
	cmp eax,0
	jl .noaff
	push dword l3
	push dword l2
	push dword l1
	call polygouro
	add esp,12
.noaff:
	popad
	retn
	
[GLOBAL coupe]
H EQU (32<<8)
;void coupe (vecic *p1, vecic *p2, vecic *pr) {
;	pr->x=p1->x+( ( (H-p1->z) * (((p2->x-p1->x)<<8)/(p2->z-p1->z)) )>>8 );
;	pr->y=p1->y+( ( (H-p1->z) * (((p2->y-p1->y)<<8)/(p2->z-p1->z)) )>>8 );
;	pr->z=H;
;}
coupe:
	pushad
	mov ebp,[esp+32+4+8]	; pointe sur pr
	mov esi,[esp+32+4]	; pointe sur p1
	mov edi,[esp+32+4+4]	; pointe sur p2
	mov ebx,[edi+8]	; ebx=z2
	mov ecx,H
	mov eax,[edi]		; eax=x2
	sub ebx,[esi+8]	; ebx=z2-z1
	sub eax,[esi]		; eax=x2-x1
	sub ecx,[esi+8]	; ecx=H-z1
	mov edx,eax
	shl eax,8			; edx:eax = (x2-x1)<<8
	sar edx,(32-8)
	idiv ebx				; eax=(x2-x1)<<8/(z2-z1)
	imul ecx
	shrd eax,edx,8
	add eax,[esi]
	mov [DS:ebp],eax
	
	mov eax,[edi+4]
	sub eax,[esi+4]
	mov edx,eax
	shl eax,8
	sar edx,(32-8)
	idiv ebx
	imul ecx
	shrd eax,edx,8
	add eax,[esi+4]
	mov dword [DS:ebp+8],H
	mov [DS:ebp+4],eax
	
	mov eax,[esi+12]	; en priant pour qu'on puisse aussi loader le 4eme octet
	mov [DS:ebp+12],ax
	shr eax,16
	mov [DS:ebp+14],al
	popad
	retn

[GLOBAL MMXPlotEtoile]
; void MMXPlotEtoile(int ady, int ce)
MMXPlotEtoile:
	push edi
	push eax
	mov edi,[esp+12]		;ady
	mov eax,[esp+12+4]	; --regebe
	mov [edi],eax
	pop eax
	pop edi
	retn

[GLOBAL MMXPhongInit]
; void MMXPhongInit(int aa,int intcol);
MMXPhongInit:
	push eax
	mov eax,[esp+4+4]	; aa
	mov [aa],eax
	mov [aa+4],eax
	mov eax,[esp+4+8]	; intcol
	and eax,0FFh;
	shl eax,3			; sert d'index dans la table de preca
	mov [intcol],eax
	fsave [bufferfpu]	; a partir de maintenant, plus de FPU !
	pop eax
	retn


[GLOBAL MMXPhongLumTransp]
MMXPhongLumTransp:
	pushad
	mov edi,[esp+32+4]	; dest
	mov ecx,[esp+32+8]	; size
	mov ebx,[esp+32+12]	; l
	mov edx,[esp+32+16]	; atmp
	mov esi,[esp+32+20]	; k
	mov eax,[esp+32+24]	; c
	mov ebp,[aa]
	mov dword [val],eax
	mov dword [val+4],eax
	movq mm1,qword [val]			; mm1 = 00rcgcbc00rcgcbc
	mov dword [val],ebx
	add ebx,edx
	add ebx,esi
	mov dword [val+4],ebx
	movq mm2,qword [val]			; mm2 = L2L2l2l2L1L1l1l1
	add ebx,edx
	add ebx,esi
	add ebx,ebp
	mov [val],ebx
	add ebx,edx
	add ebx,esi
	add ebx,ebp
	add ebx,ebp
	mov [val+4],ebx
	movq mm6,[val]					; mm6 = L4L4l4l4L3L3l3l3	; NE SERT PLUS A RIEN NON PLUS !!
	mov [val],edx
	add edx,ebp
	mov [val+4],edx
	movq mm3,[val]			; mm3 = (atmp2)(atmp1)
	pslld mm3,1					; x2
	movq mm4,[aa]			; mm4 = (aa)(aa)
	mov [val],esi
	mov [val+4],esi
	movq mm5,[val]			; mm5 = (k)(k)
	pslld mm5,1						; x2
	paddd mm5,mm4			; mm5 = (2*k+aa)(2*k+aa)
	pslld mm4,2				; mm4 = 4x(aa)(aa)
	mov edx,[preca]
	add edx,[intcol]
	sub edi,8
	shr ecx,1
	jz .lz
.ld:
	movq mm0,mm2				; mm0 = L2L2l2l2L1L1l1l1
	;
	packuswb mm0,mm0			; mm0 = L2l2L1l1L2l2L1l1 saturé
	add edi,8
	movd eax,mm0
	movd ebx,mm0
	shr eax,8
	and ebx,0FF000000h
	shr ebx,24
	and eax,0FFh
	paddd mm2,mm3
	movq mm0,[edx+eax*8]			; mm0 = ------i1--i1--i1
	paddd mm3,mm4
	packuswb mm0,[edx+ebx*8]	; mm0 = --i2i2i2--i1i1i1
	paddusb mm0,mm1				; mm0 = --r2g2b2--r1g1b1
	paddusb mm0,[edi]
	paddd mm2,mm5
	movq [edi],mm0
	dec ecx
	jnz .ld
.lz:
	test byte [esp+32+8],1
	jz .lzz
	movq mm0,mm2				; mm0 = L2L2l2l2L1L1l1l1
	packuswb mm0,mm0			; mm0 = L2l2L1l1L2l2L1l1 saturé
	movd eax,mm0
	shr eax,8
	and eax,0FFh
	movq mm0,[edx+eax*8]			; mm0 = ------i1--i1--i1
	packuswb mm0,mm0				; mm0 = --i1i1i1--i1i1i1
	paddusb mm0,mm1				; mm0 = --r1g1b1--r1g1b1
	paddusb mm0,[edi+8]
	movd [edi+8],mm0
.lzz:
	popad
	retn
	
	
[GLOBAL MMXPhongLum]
; void MMXPhongLum(int dest, nbre de pixels, l, atmp, k, int c);
;					for (; j!=jlim; j++) {
;						int cc;
;						cc=l<(1<<24)?preca[l>>16]:0;
;						vid[j].r=(int)c.r+(int)cc<256?c.r+cc:255;
;						vid[j].g=(int)c.g+(int)cc<256?c.g+cc:255;
;						vid[j].b=(int)c.b+(int)cc<256?c.b+cc:255;
;                 l+=k+atmp;
;                 atmp+=aa;
;					}
MMXPhongLum:
	pushad
	mov edi,[esp+32+4]	; dest
	mov ecx,[esp+32+8]	; size
	mov ebx,[esp+32+12]	; l
	mov edx,[esp+32+16]	; atmp
	mov esi,[esp+32+20]	; k
	mov eax,[esp+32+24]	; c
	mov ebp,[aa]
	mov dword [val],eax
	mov dword [val+4],eax
	movq mm1,qword [val]			; mm1 = 00rcgcbc00rcgcbc
	mov dword [val],ebx
	add ebx,edx
	add ebx,esi
	mov dword [val+4],ebx
	movq mm2,qword [val]			; mm2 = L2L2l2l2L1L1l1l1
	add ebx,edx
	add ebx,esi
	add ebx,ebp
	mov [val],ebx
	add ebx,edx
	add ebx,esi
	add ebx,ebp
	add ebx,ebp
	mov [val+4],ebx
	movq mm6,[val]					; mm6 = L4L4l4l4L3L3l3l3	; NE SERT PLUS A RIEN !!
	mov [val],edx
	add edx,ebp
	mov [val+4],edx
	movq mm3,[val]			; mm3 = (atmp2)(atmp1)
	pslld mm3,1					; x2
	movq mm4,[aa]			; mm4 = (aa)(aa)
	mov [val],esi
	mov [val+4],esi
	movq mm5,[val]			; mm5 = (k)(k)
	pslld mm5,1						; x2
	paddd mm5,mm4			; mm5 = (2*k+aa)(2*k+aa)
	pslld mm4,2				; mm4 = 4x(aa)(aa)
	mov edx,[preca]
	add edx,[intcol]			; gruge pour réduire la lumière
	sub edi,8
	shr ecx,1
	jz .lz
.ld:
	movq mm0,mm2				; mm0 = L2L2l2l2L1L1l1l1
	;
	packuswb mm0,mm0			; mm0 = L2l2L1l1L2l2L1l1 saturé
	add edi,8
	movd eax,mm0
	movd ebx,mm0
	shr eax,8
	and ebx,0FF000000h
	shr ebx,24
	and eax,0FFh
	paddd mm2,mm3
	movq mm0,[edx+eax*8]			; mm0 = ------i1--i1--i1
	paddd mm3,mm4
	packuswb mm0,[edx+ebx*8]	; mm0 = --i2i2i2--i1i1i1
	paddusb mm0,mm1				; mm0 = --r2g2b2--r1g1b1
	paddd mm2,mm5
	movq [edi],mm0
	dec ecx
	jnz .ld
.lz:
	test byte [esp+32+8],1
	jz .lzz
	movq mm0,mm2				; mm0 = L2L2l2l2L1L1l1l1
	packuswb mm0,mm0			; mm0 = L2l2L1l1L2l2L1l1 saturé
	movd eax,mm0
	shr eax,8
	and eax,0FFh
	movq mm0,[edx+eax*8]			; mm0 = ------i1--i1--i1
	packuswb mm0,mm0				; mm0 = --i1i1i1--i1i1i1
	paddusb mm0,mm1				; mm0 = --r1g1b1--r1g1b1
	movd [edi+8],mm0
.lzz:
	popad
	retn
	
[GLOBAL MMXFlatInit]
; void MMXPhongInit();
MMXFlatInit:
	fsave [bufferfpu]
	retn
	
[GLOBAL MMXSaveFPU]
MMXSaveFPU:
	fsave [bufferfpu]
	retn

[GLOBAL MMXAddSatC]
MMXAddSatC:
	push eax
	push ebx
	mov ebx,[esp+8+4]
	movd mm0,[ebx]	; movq
	mov eax,[esp+8+8]
	mov [val],eax
	mov [val+4],eax
	paddusb mm0,[val]
	movd [ebx],mm0	; movq normalement pour 2 words
	pop ebx
	pop eax
	retn

[GLOBAL MMXAddSat]
MMXAddSat:
	push eax
	push ebx
	mov ebx,[esp+8+4]
	movd mm0,[ebx]	; movq
	mov eax,[esp+8+8]
	mov ah,al
	mov [val],eax
	mov [val+2],al
	paddusb mm0,[val]
	movd [ebx],mm0	; movq normalement pour 2 words
	pop ebx
	pop eax
	retn


	
[GLOBAL MMXSubSat]
MMXSubSat:
	push eax
	push ebx
	mov ebx,[esp+8+4]
	movd mm0,[ebx]	; movq
	mov eax,[esp+8+8]
	mov ah,al
	mov [val],eax
	mov [val+2],al
;	mov [val+4],eax
;	mov [val+6],al
	psubusb mm0,[val]
	movd [ebx],mm0	; movq normalement pour 2 words
	pop ebx
	pop eax
	retn
	
[GLOBAL MMXRestoreFPU]
MMXRestoreFPU:
	emms
	frstor [bufferfpu]
	retn
	
[GLOBAL MMXFlat]
; void MMXFlat(int dest, nbre de pixels, int c);
MMXFlat:
	pushad
	mov edi,[esp+32+4]	; dest
	mov ecx,[esp+32+8]	; size
	mov eax,[esp+32+12]	; c
	mov dword [val],eax
	mov dword [val+4],eax
	movq mm1,qword [val]			; mm1 = 00rcgcbc00rcgcbc
	shr ecx,1
	jz .lz
.ld:
	movq [edi],mm1
	add edi,8
	dec ecx
	jnz .ld
.lz:
	test byte [esp+32+8],1
	jz .lzz
	movd [edi],mm1
.lzz:
	popad
	retn
	
[GLOBAL MMXFlatTransp]
; void MMXFlatTransp(int dest, nbre de pixels, int c);
MMXFlatTransp:
	pushad
	mov edi,[esp+32+4]	; dest
	mov ecx,[esp+32+8]	; size
	mov eax,[esp+32+12]	; c
	mov dword [val],eax
	mov dword [val+4],eax
	movq mm1,qword [val]			; mm1 = 00rcgcbc00rcgcbc
	shr ecx,1
	jz .lz
.ld:
	movq mm0,[edi]
	paddusb mm0,mm1
	movq [edi],mm0
	add edi,8
	dec ecx
	jnz .ld
.lz:
	test byte [esp+32+8],1
	jz .lzz
	movq mm0,[edi]
	paddusb mm0,mm1
	movd [edi],mm0
.lzz:
	popad
	retn

[GLOBAL MMXMemSetInt]
; void MMXMemSetInt(int *deb, int coul, int n);
MMXMemSetInt:
	fsave [bufferfpu]
	push eax
	push edi
	push ecx
	mov edi,[esp+16]	; debut
	mov eax,[esp+20]	; coul
	mov ecx,[esp+24]	; count
	shr ecx,1
	jz .l2
	mov dword [val],eax
	mov dword [val+4],eax
	movq mm0,[val]
.l0:
	dec ecx
	movq [edi+ecx*8],mm0
	jnz .l0
.l2:
	mov ecx,[esp+24]
	shr ecx,1
	shl ecx,3
	add edi,ecx
	and byte [esp+24],1
	jz .l1
	mov [edi],eax
.l1:
	emms
	pop ecx
	pop edi
	pop eax
	frstor [bufferfpu]
	retn
	
[GLOBAL MMXBlur]
; void MMXBlur(int *deb, int size, int blured);
MMXBlur:
	fsave [bufferfpu]
	push eax
	push edi
	push ecx
	mov edi,[esp+16]	; debut
	mov ecx,[esp+20]	; count
	mov al,[esp+24]	; blured
	neg al
	mov ah,al
	mov [val],ax
	mov [val+2],ax
	mov eax,[val]
	mov [val+4],eax
	shr ecx,1
	jz .l2
	movq mm0,[val]
.l0:
	dec ecx
	movq mm1,[edi+ecx*8]
	psubusb mm1,mm0
	movq [edi+ecx*8],mm1
	jnz .l0
.l2:
	test byte [esp+24],1
	jz .l1
	mov ecx,[esp+20]
	shr ecx,1
	movd mm1,[edi+ecx*8]
	psubusb mm1,mm0
	movd [edi+ecx*8],mm1
.l1:
	emms
	pop ecx
	pop edi
	pop eax
	frstor [bufferfpu]
	retn
	
	
[GLOBAL MMXAddSatInt]
; void MMXAddSatInt(int *deb, int coul, int n);
MMXAddSatInt:
	fsave [bufferfpu]
	push eax
	push edi
	push ecx
	mov edi,[esp+16]	; debut
	mov eax,[esp+20]	; coul
	mov ecx,[esp+24]	; count
	shr ecx,1
	jz .l2
	mov dword [val],eax
	mov dword [val+4],eax
	movq mm0,[val]
.l0:
	dec ecx
	movq mm1,[edi+ecx*8]
	paddusb mm1,mm0
	movq [edi+ecx*8],mm1
	jnz .l0
.l2:
	mov ecx,[esp+24]
	shr ecx,1
	shl ecx,3
	add edi,ecx
	and byte [esp+24],1
	jz .l1
	movq mm1,[edi]
	paddusb mm1,mm0
	movd [edi],mm1
.l1:
	emms
	pop ecx
	pop edi
	pop eax
	frstor [bufferfpu]
	retn
	
[GLOBAL MMXCopyToScreen]
; void MMXCopyToScreen(int *dest, int *src, int sx, int sy, int width);
MMXCopyToScreen:
	fsave [bufferfpu]
	pushad
	mov edi,[esp+32+4]	; dest
	mov esi,[esp+32+8]	; src
	mov ebp,[esp+32+20]	; width
	cmp byte [depth],24
	jne near .no24
	; blit vers 24 bits
	movq mm6,[pix1Mask]
	movq mm7,[pix2Mask]
	lea ebp,[ebp+ebp*2]	; en octets
.l1:	
	mov ecx,[esp+32+12]	; sx
	shr ecx,3				; 8 pixels d'un coup
	jz near .l2
	push edi
.l0:
	movq mm0,[esi]			; mm0      = --r1g1b1--r0g0b0
	movq mm2,[esi+8]		; mm2      = --r3g3b3--r2g2b2
	movq mm1,mm0			; mm1      = --r1g1b1--r0g0b0
	movq mm4,[esi+16]		; mm4      = --r5g5b5--r4g4b4
	movq mm3,mm2			; mm3      = --r3g3b3--r2g2b2
	pand mm0,mm6			; mm0      = ----------r0g0b0
	pand mm1,mm7			; mm1      = --r1g1b1--------
	pand mm3,mm6			; mm3      = ----------r2g2b2
	psrlq mm1,8				; mm1      = ----r1g1b1------
	movq mm5,[esi+24]		; mm5      = --r7g7b7--r6g6b6
	por mm0,mm1				; mm0      = ----r1g1b1r0g0b0
	psrlq mm3,16			; mm3      = --------------r2
	movq mm1,mm2			; mm1      = --r3g3b3--r2g2b2
	pand mm5,mm6			; mm5      = ----------r6g6b6
	psllq mm1,48			; mm1      = g2b2------------
	pand mm2,mm7			; mm2      = --r3g3b3--------
	por mm0,mm1				; mm0      = g2b2r1g1b1r0g0b0
	psrlq mm2,24			; mm2      = --------r3g3b3--
	movq [edi],mm0			; [edi+00] = g2b2r1g1b1r0g0b0
	por mm3,mm2				; mm3      = --------r3g3b3r2
	psllq mm5,16			; mm5      = ------r6g6b6----
	movq mm2,mm4			; mm2      = --r5g5b5--r4g4b4
	add edi,24
	pand mm4,mm7			; mm4      = --r5g5b5--------
	pand mm2,mm6			; mm2      = ----------r4g4b4
	psllq mm2,32			; mm2      = --r4g4b4--------
	por mm3,mm2				; mm3      = --r4g4b4r3g3b3r2
	movq mm2,mm4			; mm2      = --r5g5b5--------
	add esi,32
	psrlq mm4,40			; mm4      = ------------r5g5
	por mm4,mm5				; mm4      = ------r6g6b6r5g5
	movd mm5,[esi-4]		; mm5      = ----------r7g7b7
	psllq mm2,24			; mm2      = b5--------------
	por mm3,mm2				; mm3      = b5r4g4b4r3g3b3r2
	movq [edi-16],mm3		; [edi+08] = b5r4g4b4r3g3b3r2
	psllq mm5,40			; mm5      = r7g7b7----------
	dec ecx
	por mm4,mm5				; mm4      = r7g7b7r6g6b6r5g5
	movq [edi-8],mm4		; [edi+16] = r7g7b7r6g6b6r5g5
	
	jnz near .l0
	; finir la ligne
.l2:
	mov ecx,[esp+32+12]	; sx	
	and ecx,111b
	jz .l3
.l4:
	mov eax,[esi]
	add esi,4
	mov [edi],eax
	add edi,3
	dec ecx
	jnz .l4
.l3:
	pop edi
	add edi,ebp			; width
	dec dword [esp+32+16]		; sy
	jnz near .l1
.exit:	
	emms
	popad
	frstor [bufferfpu]
	retn
.no24:
	cmp byte [depth],32
	jne .dep16
	shl ebp,2	; width en octets
.l21:	
	mov ecx,[esp+32+12]	; sx
	shr ecx,2				; 4 pixels d'un coup
	jz near .l22
	push edi
.l20:
	movq mm0,[esi]			; mm0      = --r1g1b1--r0g0b0
	movq mm2,[esi+8]		; mm2      = --r3g3b3--r2g2b2
	movq [edi],mm0			; [edi+00]
	movq [edi+8],mm2		; [edi+08] = b5r4g4b4r3g3b3r2
	add edi,16
	add esi,16
	dec ecx
	jnz near .l20
.l22:
	mov ecx,[esp+32+12]	; sx	
	and ecx,11b
	jz .l23
.l24:
	mov eax,[esi]
	add esi,4
	mov [edi],eax
	add edi,4
	dec ecx
	jnz .l24
.l23:
	pop edi
	add edi,ebp			; width
	dec dword [esp+32+16]		; sy
	jnz near .l21
	jmp .exit
.dep16:
	add ebp,ebp		; width en octets
.l31:
	mov ecx,[esp+32+12]	; sx
	push edi
	dec ecx
	and ecx,0fffffff8h
	movq mm7,[rgbMulFactor]
	movq mm6,[rgbMask2]
	movq mm2,[esi+ecx*4+8]
	movq mm0,[esi+ecx*4]
	movq mm3,mm2
	pand mm3,[rgbMask1]
	movq mm1,mm0
	pand mm1,[rgbMask1]
	pmaddwd mm3,mm7
	pmaddwd mm1,mm7
	pand mm2, mm6
.l30:
	movq mm4,[esi+ecx*4+24]
	pand mm0,mm6
	movq mm5,[esi+ecx*4+16]
	por mm3,mm2
;	psrld mm3,5
	pslld mm3,11
	psrad mm3,16
	por mm1,mm0
	movq mm0,mm4
;	psrld mm1,5
	pslld mm1,11
	psrad mm1,16
	pand mm0,[rgbMask1]
	packssdw mm1,mm3
	movq mm3,mm5
	pmaddwd mm0,mm7
	pand mm3,[rgbMask1]
	pand mm4,mm6
	movq [edi+ecx*2],mm1
	pmaddwd mm3,mm7
	sub ecx,8
	por mm4,mm0
	pand mm5,mm6
;	psrld mm4,5
	pslld mm4,11
	psrad mm4,16
	movq mm2,[esi+ecx*4+8]
	por mm5,mm3
	movq mm0,[esi+ecx*4]
;	psrld mm5,5
	pslld mm5,11
	psrad mm5,16
	movq mm3,mm2
	movq mm1,mm0
	pand mm3,[rgbMask1]
	packssdw mm5,mm4
	pand mm1,[rgbMask1]
	pand mm2,mm6
	movq [edi+ecx*2+24],mm5
	pmaddwd mm3,mm7
	pmaddwd mm1,mm7
	jge near .l30
	pop edi
	mov eax,dword [esp+32+12]	; sx
	shl eax,2
	add esi,eax
	add edi,ebp			; width
	dec dword [esp+32+16]		; sy
	jnz near .l31
	jmp .exit

	
	
[GLOBAL MMXCopy]
; void MMXCopy(int *dest, int *src, int nbr);
MMXCopy:
	fsave [bufferfpu]
	pushad
	mov edi,[esp+32+4]	; dest
	mov esi,[esp+32+8]	; src
	mov ecx,[esp+32+12]	; count
	shl ecx,2
	sub ecx,8			; on assume que c'est multiple de 8
	add edi,8
.l0:	movq mm0,qword [esi+ecx]
	sub ecx,8
	movq qword [edi+ecx],mm0
	jns .l0
	emms
	popad
	frstor [bufferfpu]
	retn

[GLOBAL MMXGouro]
MMXGouro:
	push eax
	push ecx
	push edx
	mov eax,[Gouroqb]
	mov [val],ax
	mov eax,[Gouroqg]
	mov [val+2],ax
	mov eax,[Gouroqr]
	mov [val+4],ax
	movq mm7,[val]		; mm7 = Gouroqrgb
	
	cmp dword [Gouroyi],0
	jge near .finif0
	mov eax,[Gourody]
	neg eax
	cmp [Gouroyi],eax
	jge .else
	neg eax
	add [Gouroyi],eax
	mov edx,eax
	imul eax,[Gouroqx]
	add [Gouroxi],eax
	mov eax,edx
	imul eax,[Gouroql]
	add [Gourolx],eax
	mov [val],edx
	mov [val+2],dx
	mov [val+4],dx
	movq mm2,[val]
	pmullw mm2,mm7
	paddw mm6,mm2
	jmp .return
.else:
	mov eax,[Gouroyi]
	add [Gourody],eax
	mov edx,eax
	mov [val],eax
	mov [val+2],ax
	mov [val+4],ax
	movq mm2,[val]
	imul eax,[Gouroqx]
	sub [Gouroxi],eax
	imul edx,[Gouroql]
	sub [Gourolx],edx
	pmullw mm2,mm7
	psubw mm6,mm2
	mov dword [Gouroyi],0
.finif0:
	mov eax,[Gouroyi]
	imul eax,[SX]
	lea edx,[eax*4]
	add edx,[videobuffer]
	mov [Gourovid],edx
	
	cmp dword [Gourody],0
	jle near .finwhile
.w:
	mov eax,[SY]
	cmp [Gouroyi],eax
	jge near .finwhile
	mov ecx,[Gouroxi]
	sar ecx,8	; ecx=i
	mov eax,[Gourolx]
	sar eax,8
	mov edx,ecx
	sub edx,eax	; edx=ilim
	jns .l0
	xor edx,edx
.l0:
	movq mm1,mm6	; mm1=crgb=Gourocoulrgb
	cmp ecx,[SX]
	jl .l1
	mov eax,ecx
	mov ecx,[SX]
	dec ecx
	sub eax,ecx
	mov [val],eax
	mov [val+2],ax
	mov [val+4],ax
	movq mm2,[val]
	pmullw mm2,mm0
	paddw mm1,mm2
.l1:
	cmp ecx,edx
	jl .finif
	mov eax,ecx
	sub eax,edx
	shl edx,2
	inc eax
	add edx,[Gourovid]
	mov cl,al
	movq mm2,mm1
	shr eax,1
	paddw mm2,mm0			; mm2=c+i
	jz .l00
	jnc .l20
	add edx,4
.l20:
	movq mm3,mm1
	movq mm4,mm2
	psrlw mm3,8
	paddw mm1,mm5
	psrlw mm4,8
	dec eax
	packuswb mm4,mm3
	paddw mm2,mm5
	movq qword [edx+eax*8],mm4
	jnz .l20
	sub edx,4
.l00:
	test cl,1
	jz .l10
.l30:
	psrlw mm1,8
	packuswb mm1,mm1
	movd dword [edx],mm1
.l10:
;	mov dword [edx],0	; pour voir les polys
.finif:
	mov eax,[Gouroqx]
	mov edx,[Gouroql]
	add [Gouroxi],eax
	add [Gourolx],edx
	paddw mm6,mm7
	mov eax,[SX]
	shl eax,2
	add [Gourovid],eax
	inc dword [Gouroyi]
	dec dword [Gourody]
	jnz near .w
.finwhile:
.return:	
	pop edx
	pop ecx
	pop eax
	retn
	
[GLOBAL MMXGouroPreca]
MMXGouroPreca:
	push eax
	mov dword [val+4],0
	mov eax,[esp+4+4]	; ib
	mov word [GouroI],ax
	mov eax,[esp+4+8]	; ig
	mov word [GouroI+2],ax
	mov eax,[esp+4+12]	; ir
	mov word [GouroI+4],ax
	movq mm0,[GouroI]		; mm0=i
	movq mm5,mm0
	paddw mm5,mm0			; mm5=2*i
	mov eax,[Gourocoulb]
	mov [val],ax
	mov eax,[Gourocoulg]
	mov [val+2],ax
	mov eax,[Gourocoulr]
	mov [val+4],ax
	movq mm6,[val]		; mm6 = Gourocoulrgb, qui ne change pas pour tout le triangle
	pop eax
	retn

[SEGMENT .data]
[EXTERN keytab]
pix1Mask		dd 0FFFFFFh,0
pix2Mask		dd 0,0FFFFFFh
GouroGrey	dd 00808080h,00808080h
BckBck		db 0,224,224,0,0,224,224,0
BckCiel		dd 0A0A0C0h,0A0A0C0h
rgbMulFactor	dd 20000004h,20000004h	;8 au lieu de 4
rgbMask1			dd 00F800f8h,00F800f8h
rgbMask2			dd 0000fc00h,0000fc00h	;f8 au lieu de fc
[GLOBAL BigFont]
BigFont		incbin "bigfont.comp"
[GLOBAL font]
font			incbin "font.comp"
[SEGMENT .bss]
GouroI	resq 1
iy			resd 1
ix			resd 1
val		resq 1
val2		resq 1
aa			resd 2	; des qwords en fait
newyoff	resb 4
idintltmp	resd 1
idinthtmp	resd 1
l1			resd 3	; des vect2dc
l2			resd 3
l3			resd 3
bufferfpu	resb 108
[GLOBAL Chronol]
[GLOBAL Chronoh]
[EXTERN preca]
[COMMON _DX 4]
[COMMON _DY 4]
[COMMON SX 4]
[COMMON SY 4]
[COMMON Gouroxi 4]
[COMMON Gouroyi 4]
[COMMON Gourolx 4]
[COMMON Gouroql 4]
[COMMON Gouroqx 4]
[COMMON Gouroqr 4]
[COMMON Gouroqg 4]
[COMMON Gouroqb 4]
[COMMON Gourocoulr 4]
[COMMON Gourocoulg 4]
[COMMON Gourocoulb 4]
[COMMON Gourovid 4]
[COMMON Gourody 4]
[COMMON Gouroir 4]
[COMMON Gouroig 4]
[COMMON Gouroib 4]
[COMMON videobuffer 4]
[COMMON zfac 4]
[COMMON depth 4]
[EXTERN coulpoly]
Chronol		resd 1
Chronoh		resd 1
cltmp			resd 1
chtmp			resd 1
intcol		resd 1
