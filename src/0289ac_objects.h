/* 8c0289ac */
#ifndef _0289AC_OBJECTS_H
#define _0289AC_OBJECTS_H

#include <shinobi.h>

void ObjectsInitPedestrianGroups_8c0296d6(void);
void ObjectsFreePedestrianGroups_8c0297da(void);
void ObjectsInitBlinkers_8c029920(void);
void ObjectsClearAssetRequestTable_8c029acc(void);
void ObjectsFreeAssetRequests_8c029cfe(void);
void ObjectsRelocatePedGroupLists_8c028dd0(void *handle);
void ObjectsRelocatePedGroupDefs_8c028de8(void *handle);
void ObjectsStartAssetRequests_8c029ad4(int *table);
void ObjectsPushTasks_8c02a6ac(void);

extern NJS_TEXANIM init_pedestrianTexAnims_8c04623c[];

#endif // _0289AC_OBJECTS_H
