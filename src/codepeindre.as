[SEGMENT .text]
[EXTERN main]

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
;	mov [val+4],eax
;	mov [val+6],al
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
	lea ebp,[ebp+ebp*2]	; en octets
	movq mm6,[pix1Mask]
	movq mm7,[pix2Mask]
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
	
	emms
	popad
	frstor [bufferfpu]
	retn

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

[SEGMENT .data]
pix1Mask		dd 0FFFFFFh,0
pix2Mask		dd 0,0FFFFFFh
[SEGMENT .bss]
bufferfpu	resb 108
val		resq 1
aa			resd 2	; des qwords en fait
idintltmp	resd 1
idinthtmp	resd 1
l1			resd 3	; des vect2dc
l2			resd 3
l3			resd 3
[EXTERN preca]
[COMMON _DX 4]
[COMMON _DY 4]
[COMMON SX 4]
[COMMON SY 4]
[COMMON videobuffer 4]
[EXTERN coulpoly]
cltmp			resd 1
chtmp			resd 1
intcol		resd 1
