
/*
 * Copyright (C) lcinx
 * lcinx@163.com
*/
#pragma once
#include "platform_config.h"

#ifdef __cplusplus
extern "C"{
#endif
	
struct idmgr;

/* 
 * maxid is total, valid id is 1 --- maxid.
 * delaytime is delay release time, when call idmgr_freeid, delay delaytime later, real free.
 * */
inline struct idmgr *idmgr_create (int maxid, int delaytime);

inline int idmgr_total (struct idmgr *self);

inline int idmgr_usednum (struct idmgr *self);

inline void idmgr_release (struct idmgr *self);

inline int idmgr_allocid (struct idmgr *self);

inline bool idmgr_freeid (struct idmgr *self, int id);

inline void idmgr_run (struct idmgr *self);

#ifdef __cplusplus
}
#endif