#pragma once

#define AOI_RADIS 200.0f
#define INVALID_ID (~0)
#define PRE_ALLOC 16
#define AOI_RADIS2 (AOI_RADIS * AOI_RADIS)
#define LEAVE_AOI_RADIS2 ((AOI_RADIS2) * 4)
#define DIST2(p1,p2) ((p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]) + (p1[2] - p2[2]) * (p1[2] - p2[2]))
#define MODE_WATCHER 1
#define MODE_MARKER 2
#define MODE_MOVE 4
#define MODE_DROP 8

#define MAX_AOIMODE_LEN 4
