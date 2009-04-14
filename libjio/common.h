
/*
 * libjio - A library for Journaled I/O
 * Alberto Bertogli (albertito@blitiri.com.ar)
 *
 * Header for internal functions
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <sys/types.h>	/* for ssize_t and off_t */
#include <stdint.h>	/* for uint*_t */

#include "libjio.h"	/* for struct jfs */
#include "fiu-local.h"	/* for fault injection functions */

#define _F_READ		0x00001
#define _F_WRITE	0x00010
#define _F_LOCK		0x00100
#define _F_TLOCK	0x01000
#define _F_ULOCK	0x10000

#define F_LOCKR		(_F_LOCK | _F_READ)
#define F_LOCKW		(_F_LOCK | _F_WRITE)
#define F_TLOCKR	(_F_TLOCK | _F_READ)
#define F_TLOCKW	(_F_TLOCK | _F_WRITE)
#define F_UNLOCK	(_F_ULOCK)

#define MAX_TSIZE	(SSIZE_MAX)


off_t plockf(int fd, int cmd, off_t offset, off_t len);
ssize_t spread(int fd, void *buf, size_t count, off_t offset);
ssize_t spwrite(int fd, const void *buf, size_t count, off_t offset);
int get_jdir(const char *filename, char *jdir);
void get_jtfile(struct jfs *fs, unsigned int tid, char *jtfile);

int checksum(int fd, size_t len, uint32_t *csum);
uint32_t checksum_map(uint8_t *map, size_t count);

void autosync_check(struct jfs *fs);

#endif
