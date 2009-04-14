
/*
 * libjio - A library for Journaled I/O
 * Alberto Bertogli (albertito@blitiri.com.ar)
 */

#ifndef _LIBJIO_H
#define _LIBJIO_H

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <pthread.h>
#include <unistd.h>

/* Check if we're using Large File Support - otherwise refuse to build.
 * Otherwise, we would allow applications not using LFS to link with the
 * library (which uses LFS) and that's just begging for problems. There should
 * be a portable way for the C library to do some of this for us, but until I
 * find one, this is the best we can do */
#ifndef _LARGEFILE_SOURCE
#error "You must compile your application with Large File Support"
#endif

/* empty declarations, the API does not expose these */
struct jlinger;
struct joper;
struct autosync_cfg;

/* the main file structure */
struct jfs {
	int fd;			/* main file descriptor */
	char *name;		/* and its name */
	char *jdir;		/* journal directory */
	int jdirfd;		/* journal directory file descriptor */
	int jfd;		/* journal's lock file descriptor */
	unsigned int *jmap;	/* journal's lock file mmap area */
	uint32_t flags;		/* journal flags */
	struct jlinger *ltrans;	/* lingered transactions */
	size_t ltrans_len;	/* length of all the lingered transactions */
	pthread_mutex_t ltlock;	/* lingered transactions' lock */
	pthread_mutex_t lock;	/* a soft lock used in some operations */
	struct autosync_cfg *as_cfg; /* autosync config */
};

/* a transaction */
struct jtrans {
	struct jfs *fs;		/* journal file structure to operate on */
	int id;			/* transaction id */
	uint32_t flags;		/* transaction flags */
	unsigned int numops;	/* quantity of operations in the list */
	size_t len;		/* transaction's length */
	pthread_mutex_t lock;	/* used to modify the operation list */
	struct joper *op;	/* list of operations */
};

struct jfsck_result {
	int total;		/* total transactions files we looked at */
	int invalid;		/* invalid files in the journal directory */
	int in_progress;	/* transactions in progress */
	int broken;		/* transactions broken */
	int corrupt;		/* corrupt transactions */
	int apply_error;	/* errors applying the transaction */
	int reapplied;		/* transactions that were reapplied */
};


/* core functions */
int jopen(struct jfs *fs, const char *name, int flags, int mode, int jflags);
void jtrans_init(struct jfs *fs, struct jtrans *ts);
int jtrans_add(struct jtrans *ts, const void *buf, size_t count, off_t offset);
ssize_t jtrans_commit(struct jtrans *ts);
ssize_t jtrans_rollback(struct jtrans *ts);
void jtrans_free(struct jtrans *ts);
int jsync(struct jfs *fs);
int jmove_journal(struct jfs *fs, const char *newpath);
int jclose(struct jfs *fs);

/* autosync */
int jfs_autosync_start(struct jfs *fs, time_t max_sec, size_t max_bytes);
int jfs_autosync_stop(struct jfs *fs);


/* journal checker */
int jfsck(const char *name, const char *jdir, struct jfsck_result *res);
int jfsck_cleanup(const char *name, const char *jdir);

/* UNIX API wrappers */
ssize_t jread(struct jfs *fs, void *buf, size_t count);
ssize_t jpread(struct jfs *fs, void *buf, size_t count, off_t offset);
ssize_t jreadv(struct jfs *fs, const struct iovec *vector, int count);
ssize_t jwrite(struct jfs *fs, const void *buf, size_t count);
ssize_t jpwrite(struct jfs *fs, const void *buf, size_t count, off_t offset);
ssize_t jwritev(struct jfs *fs, const struct iovec *vector, int count);
int jtruncate(struct jfs *fs, off_t length);
off_t jlseek(struct jfs *fs, off_t offset, int whence);

/* ANSI C stdio wrappers */
struct jfs *jfopen(const char *path, const char *mode);
int jfclose(struct jfs *stream);
struct jfs *jfreopen(const char *path, const char *mode, struct jfs *stream);
size_t jfread(void *ptr, size_t size, size_t nmemb, struct jfs *stream);
size_t jfwrite(const void *ptr, size_t size, size_t nmemb, struct jfs *stream);
int jfileno(struct jfs *stream);
int jfeof(struct jfs *stream);
void jclearerr(struct jfs *stream);
int jferror(struct jfs *stream);
int jfseek(struct jfs *stream, long offset, int whence);
long jftell(struct jfs *stream);
void jrewind(struct jfs *stream);
FILE *jfsopen(struct jfs *stream, const char *mode);


/* jfs and jtrans constants */
#define J_NOLOCK	1	/* don't lock the file before operating on it */
#define J_NOROLLBACK	2	/* no need to read rollback information */
#define J_LINGER	4	/* use lingering transactions */
#define J_COMMITTED	8	/* mark a transaction as committed */
#define J_ROLLBACKED	16	/* mark a transaction as rollbacked */
#define J_ROLLBACKING	32	/* mark a transaction as rollbacking */
#define J_RDONLY	64	/* mark a file as read-only */

/* jfsck constants (return values) */
#define J_ESUCCESS	0	/* success - shouldn't be used */
#define J_ENOENT	-1	/* no such file */
#define J_ENOJOURNAL	-2	/* no journal associated */
#define J_ENOMEM	-3	/* no enough free memory */


#endif
