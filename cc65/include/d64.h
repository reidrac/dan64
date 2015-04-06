/*
 * D64 system calls
 */
#ifndef _D64_H
#define _D64_H

#include <stdint.h>

extern void __fastcall__ sys_exit(uint8_t code);
extern uint8_t __fastcall__ sys_load(uint8_t *dest);
extern uint8_t __fastcall__ sys_save(uint8_t *src, uint16_t size);
extern uint8_t sys_ver();

extern void __fastcall__ exit(int code);

extern uint8_t __fastcall__ putch(char c);
extern uint8_t __fastcall__ cputs(char *zstr);
extern uint8_t __fastcall__ gotoxy(uint8_t x, uint8_t y);
extern uint8_t clrscr();
extern uint8_t __fastcall__ fillscr(char c);
extern char getch();
extern uint8_t __fastcall__ cgets(char *dest, uint8_t size);
extern uint8_t __fastcall__ putt(uint8_t *tile);
extern uint8_t _rand();
extern uint8_t _srand(uint16_t seed);
extern void wait_vsync();

#endif // _D64_H

