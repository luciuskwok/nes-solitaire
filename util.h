// util.h
//
// C header for the ASM util functions

unsigned char __fastcall__ readJoypad(void);
unsigned char __fastcall__ pseudorandom(void);
void __fastcall__ seedrandom(unsigned int x);
void __fastcall__ updateVramFast(void);

#define FamiTone_Enable 0
#if FamiTone_Enable
void __fastcall__ initFamiTone(void);
void __fastcall__ initFamiToneSfx(void);
#endif 

