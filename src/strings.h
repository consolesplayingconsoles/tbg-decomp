#ifndef STRINGS_H
#define STRINGS_H

#ifdef GAME_LANG_EN
#include "strings_en_us.h"
#else
#include "strings_ja_jp.sjis.h"
#endif

/* Wraps the explicit length of a message buffer sized to the archived asm's
 * Shift-JIS bytes plus its trailing padding. English strings don't share that
 * layout, so the compiler sizes them instead. */
#ifdef GAME_LANG_EN
#define TEXT_SJIS_SIZE(n)
#else
#define TEXT_SJIS_SIZE(n) n
#endif

#endif /* STRINGS_H */
