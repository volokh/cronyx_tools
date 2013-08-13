/*
 * Copyright (C) 2006-2008 Cronyx Engineering.
 * Author: Leo Yuriev <ly@cronyx.ru>
 *
 * This software is distributed with NO WARRANTIES, not even the implied
 * warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors grant any other persons or organisations permission to use
 * or modify this software as long as this message is kept with the software,
 * all derivative works or modified versions.
 *
 * $Id: sconfig.h,v 1.26 2009-09-04 17:10:37 ly Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sysmacros.h>
#include <net/if.h>
#include <dirent.h>

#include "cserial.h"
#ifdef CRONYX_LYSAP
/*
 * The few lines of code for LYSAP support.
 * Copyright (C) 2004-2008 Cronyx Engineering.
 * Author: Leo Yuriev <ly@cronyx.ru>
*/
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include "lysap-linux.h"
	extern char flag_lysap_info, flag_lysap_fibers_info;
#endif /* CRONYX_LYSAP */

#define SCONFIG_HEADER_MODEM	1
#define SCONFIG_HEADER_STATCHAN	2
#define SCONFIG_HEADER_STATE1	4
#define SCONFIG_HEADER_STATE3	8

extern int binder_fd, header_flags;
extern int global_error_code;
extern char vflag, eflag, sflag, mflag, cflag, fflag, iflag, flag_show_all, xflag, tflag, uflag,
	flag_roadmap, flag_quiet, silently_failed_get_request, list_mode;

extern char *optarg;
extern int optind;

#define CRONYX_DEVNODE_HIVE	"/dev/cronyx"
extern const char binder_devnode[];
extern const char devnode_hive[];

extern struct cronyx_item_info_t *binder_roadmap;
extern int binder_evolution, roadmap_items;
struct cronyx_item_info_t *live_minors[CRONYX_MINOR_MAX];

struct getset_param_t {
	long value, extra;
	char valid;
};

void open_binder (void);
int load_roadmap (void);
void hivedir_update (int can_ignore_errors);
int cronyx_set (int target_id, int param_id, ...);
int cronyx_get (int target_id, int param_id, ...);
void cronyx_get_param (int id, struct getset_param_t *param, int param_id);
void usage (void);
const char *lookup_ioctl_name (long code);
int cronyx_ioctl (int fd, long code, void *arg, int canfail);
u32 scan_timeslots (char *s, char **end);
char *format_timeslots (u32 s);
void print_modems (struct cronyx_item_info_t *node);
void print_huge (u64 val);
void print_stats_rs (struct cronyx_item_info_t *node);
void clear_stats (int id);
const char *format_e1_status (u32 status, int loop_mode);
void print_frac (u32 numerator, u32 divider);
void print_e1_stats (struct cronyx_item_info_t *node);
char *format_e3_cv (u32 cv, u32 baud, u32 time);
void print_e3_stats (struct cronyx_item_info_t *node);
void failed (const char *reason) __attribute__ ((noreturn));
int get_switch (char *line, const char *variants, ...);
int is_cmd_param (const char *param, const char *arg);
int get_bool_switch (char *line);
int get_ledmode (char *line, u32 *cadence);

void process_item (struct cronyx_item_info_t *node);
void print_chan (struct cronyx_item_info_t *node);
void setup_chan (struct cronyx_item_info_t *node, int argc, char **argv);
int print_roadmap (int parent, int level);
void print_item_name (struct cronyx_item_info_t *node, int width);
void list_items ();
