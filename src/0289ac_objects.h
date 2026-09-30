/* 8c0289ac */
#ifndef _0289AC_OBJECTS_H
#define _0289AC_OBJECTS_H

#include <shinobi.h>

void ObjectsInitPedestrianGroups_8c0296d6(void);
void ObjectsFreePedestrianGroups_8c0297da(void);
void ObjectsInitBlinkers_8c029920(void);
void ObjectsClearAssetRequestTable_8c029acc(void);
void ObjectsFreeAssetRequests_8c029cfe(void);
void ObjectsFreeMessageAssets_8c02adee(void);
void ObjectsRelocatePedGroupLists_8c028dd0(void *handle);
void ObjectsRelocatePedGroupDefs_8c028de8(void *handle);
void ObjectsStartAssetRequests_8c029ad4(int *table);
void ObjectsPushTasks_8c02a6ac(void);
void ObjectsClearMessageAssets_8c02aa28(void);
void ObjectsRequestMessageAssets_8c02aa36(void);
void ObjectsStartMessageBox_8c02ad8c(void);
void ObjectsOpenTextbox_8c02ae3e(int x, int y, float priority, int width, int height, int x2, int y2, int enable_offset);
int ObjectsSwapMessageBoxFor_8c02aefc(char *text);
int ObjectsMenuTextboxText_8c02af1c(int limit);
void ObjectsFreeTextboxes_8c02af32(void);

extern NJS_TEXANIM init_pedestrianTexAnims_8c04623c[];

#endif // _0289AC_OBJECTS_H
