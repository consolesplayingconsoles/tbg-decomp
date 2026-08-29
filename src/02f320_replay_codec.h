#include <shinobi.h>

#ifndef _02F320_REPLAY_CODEC_H_
#define _02F320_REPLAY_CODEC_H_

/* resets serializer state before ReplayCodecPack_8c02f934/ReplayCodecUnpack_8c02fa14; var_8c228ba4
 * (size in bytes of the recorded replay data) is declared in sectionB.h, this unit's storage owner */
void ReplayCodecInit_8c02f320(void);

/* referenced by 01614c_debug_menu's startReplaySave_8c016924: packs src into **dest (advancing it), size bytes */
void ReplayCodecPack_8c02f934(void *src, void **dest, Uint32 size);

/* referenced by 01614c_debug_menu's replayLoadTask_8c0169bc: unpacks src into **dest (advancing it), size bytes */
void ReplayCodecUnpack_8c02fa14(void *src, void **dest, Uint32 size);

#endif /* _02F320_REPLAY_CODEC_H_ */
