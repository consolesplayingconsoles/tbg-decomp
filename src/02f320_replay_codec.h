#include <shinobi.h>

#ifndef _02F320_REPLAY_CODEC_H_
#define _02F320_REPLAY_CODEC_H_

/* Bytes written by the last ReplayCodecPack_8c02f934 */
extern Uint32 var_replayPackedSize_8c228ba4;

/* Resets codec state */
void ReplayCodecInit_8c02f320(void);

/* Packs src into **dest (advancing it), size bytes */
void ReplayCodecPack_8c02f934(void *src, void **dest, Uint32 size);

/* Unpacks src into **dest (advancing it), size bytes */
void ReplayCodecUnpack_8c02fa14(void *src, void **dest, Uint32 size);

#endif /* _02F320_REPLAY_CODEC_H_ */
