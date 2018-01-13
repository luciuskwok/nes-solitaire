// famitone2.h

// C header to famitone2.s

// FamiToneInit uses A/X/Y registers in an unconventional way, so it needs to be called from another assembly file.
// FamiToneSfxInit uses X/Y as pointer to data.

extern void __fastcall__ FamiToneMusicStop(void);
extern void __fastcall__ FamiToneMusicPlay(unsigned char song);
extern void __fastcall__ FamiToneMusicPause(unsigned char isPaused);
extern void __fastcall__ FamiToneUpdate(void);
extern void __fastcall__ FamiToneSampleStop(void);
extern void __fastcall__ FamiToneSamplePlay(unsigned char sample);
