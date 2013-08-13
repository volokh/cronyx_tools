/*
 * Copyright (C) 2006-2009 Cronyx Engineering.
 * Author: Leo Yuriev <ly@cronyx.ru>
 *
 * This software is distributed with NO WARRANTIES, not even the implied
 * warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors grant any other persons or organisations permission to use
 * or modify this software as long as this message is kept with the software,
 * all derivative works or modified versions.
 *
 * $Id: sconfig-lib.c,v 1.54 2009-10-08 14:57:12 ly Exp $
 */

#include "sconfig.h"

const char binder_devnode[] = CRONYX_DEVNODE_HIVE "/binder";
const char devnode_hive[] = CRONYX_DEVNODE_HIVE;

struct cronyx_item_info_t *binder_roadmap;
int binder_evolution, roadmap_items;
struct cronyx_item_info_t *live_minors[CRONYX_MINOR_MAX];

static int svc2major (struct cronyx_item_info_t *item)
{
	switch (item->svc) {
		case CRONYX_SVC_DIRECT:
			return CRONYX_MJR_BINDER;
		case CRONYX_SVC_TTY_ASYNC:
			return CRONYX_MJR_ASYNC_SERIAL;
		case CRONYX_SVC_TTY_SYNC:
			return CRONYX_MJR_SYNC_SERIAL;
		default:
			return 0;
	}
}

void hivedir_provide (void)
{
	struct stat stat_info;

	if (stat (devnode_hive, &stat_info)) {
		if (errno == ENOENT) {
			if (mkdir (devnode_hive, 0755)) {
				failed (devnode_hive);
			}
		} else
			failed (devnode_hive);
	} else {
		if (!S_ISDIR (stat_info.st_mode)) {
			if (unlink (devnode_hive)) {
				failed (devnode_hive);
			}
			if (mkdir (devnode_hive, 0755)) {
				failed (devnode_hive);
			}
		}
	}

	if (stat (binder_devnode, &stat_info) == 0) {
		if (S_ISLNK (stat_info.st_mode) || !S_ISCHR (stat_info.st_mode))
			goto erase;

		if (major (stat_info.st_rdev) != CRONYX_MJR_BINDER
		    || minor (stat_info.st_rdev) != CRONYX_CONTROL_DEVICE)
			goto erase;

		return;
	      erase:
		if (unlink (binder_devnode))
			failed (binder_devnode);
	} else if (errno != ENOENT)
		failed (binder_devnode);

	if (mknod (binder_devnode, S_IFCHR | 0640, makedev (CRONYX_MJR_BINDER, CRONYX_CONTROL_DEVICE)))
		failed (binder_devnode);
}

void open_binder (void)
{
	if (binder_fd < 0) {
		hivedir_provide ();
		binder_fd = open (binder_devnode, 0);
		if (binder_fd < 0)
			failed (binder_devnode);
	}
}

static int compare_items (const void *a, const void *b)
{
	struct cronyx_item_info_t *ia = (struct cronyx_item_info_t *) a;
	struct cronyx_item_info_t *ib = (struct cronyx_item_info_t *) b;

	return ia->id - ib->id;
}

int load_roadmap (void)
{
	struct cronyx_binder_enum_t item_enum;
	int result;
	int i;

      again:
	memset (&live_minors, 0, sizeof (live_minors));
	if (binder_roadmap) {
		free (binder_roadmap);
		binder_roadmap = NULL;
	}
	roadmap_items = 0;

	memset (&item_enum, 0, sizeof (0));
	if (cronyx_ioctl (binder_fd, CRONYX_ITEM_ENUM, &item_enum, 0) < 0)
		failed ("binder.enum");

	result = item_enum.evolution == item_enum.push;
	binder_evolution = item_enum.evolution;

	if (item_enum.total > 0) {
		binder_roadmap = calloc (item_enum.total, sizeof (struct cronyx_item_info_t));
		if (binder_roadmap == 0)
			failed ("out of memory");

		roadmap_items = item_enum.total;
		for (i = 0; i < item_enum.total; i++) {
			if (i - item_enum.from == sizeof (item_enum.ids) / sizeof (item_enum.ids[0])) {
				item_enum.from = i;
				if (cronyx_ioctl (binder_fd, CRONYX_ITEM_ENUM, &item_enum, 0) < 0)
					failed ("binder.enum");
				if (binder_evolution != item_enum.evolution)
					goto again;
			}
			binder_roadmap[i].id = item_enum.ids[i - item_enum.from];
		}

		qsort (binder_roadmap, roadmap_items, sizeof (binder_roadmap[0]), compare_items);

		for (i = 0; i < roadmap_items; i++) {
			if (cronyx_ioctl (binder_fd, CRONYX_ITEM_INFO, &binder_roadmap[i], 0) < 0)
				failed ("binder.item-info");
			if (binder_roadmap[i].minor > CRONYX_CONTROL_DEVICE
			  && binder_roadmap[i].minor < CRONYX_MINOR_MAX && binder_roadmap[i].svc != CRONYX_SVC_NONE) {
				if (live_minors[binder_roadmap[i].minor] != 0)
					failed (binder_roadmap[i].name);
				live_minors[binder_roadmap[i].minor] = &binder_roadmap[i];
			}
		}
	}

	memset (&item_enum, 0, sizeof (0));
	if (cronyx_ioctl (binder_fd, CRONYX_ITEM_ENUM, &item_enum, 0) < 0)
		failed ("binder.enum");
	if (binder_evolution != item_enum.evolution)
		goto again;

	return result;
}

void bit_set (unsigned *bitmap, unsigned bit)
{
	bitmap[bit / (sizeof (unsigned) * 8)]
		|= 1u << (bit % (sizeof (unsigned) * 8));
}

int bit_test (unsigned *bitmap, unsigned bit)
{
	return (bitmap[bit / (sizeof (unsigned) * 8)]
		& (1u << (bit % (sizeof (unsigned) * 8)))) != 0;
}

#define ROUND_UP(val,factor) (((val) + (factor) - 1) / (factor))
#define BITMAP(size) ROUND_UP (size, sizeof (unsigned) * 8)

void hivedir_update (int can_ignore_errors)
{
	struct stat stat_info;
	DIR *hive;
	struct dirent *entry;
	int entry_minor, entry_major;
	struct cronyx_item_info_t *item;
	unsigned bitmap_done_names[BITMAP (CRONYX_MINOR_MAX)];
	unsigned bitmap_done_aliases[BITMAP (CRONYX_MINOR_MAX)];
	unsigned bitmap_done_names_callout[BITMAP (CRONYX_MINOR_MAX)];
	unsigned bitmap_done_aliases_callout[BITMAP (CRONYX_MINOR_MAX)];
	char entry_path[PATH_MAX + 1];

	hivedir_provide ();

	hive = opendir (devnode_hive);
	if (hive == NULL) {
		if (can_ignore_errors)
			return;
		failed (devnode_hive);
	}

	memset (&bitmap_done_names, 0, sizeof (bitmap_done_names));
	memset (&bitmap_done_aliases, 0, sizeof (bitmap_done_aliases));
	memset (&bitmap_done_names_callout, 0, sizeof (bitmap_done_names_callout));
	memset (&bitmap_done_aliases_callout, 0, sizeof (bitmap_done_aliases_callout));

	while ((entry = readdir (hive)) != NULL) {
		snprintf (entry_path, sizeof (entry_path), "%s/%s", devnode_hive, entry->d_name);
		if (stat (entry_path, &stat_info)) {
			if (errno == ENOENT)
				continue;
			if (can_ignore_errors)
				continue;
			failed (devnode_hive);
		}
		if (S_ISDIR (stat_info.st_mode)) {
			if (strcmp (entry->d_name, ".") == 0 || strcmp (entry->d_name, "..") == 0)
				continue;
			goto erase;
		}
		if (S_ISLNK (stat_info.st_mode) || !S_ISCHR (stat_info.st_mode))
			goto erase;

		entry_minor = minor (stat_info.st_rdev);
		entry_major = major (stat_info.st_rdev);
		if (entry_minor < 0 || entry_minor >= CRONYX_MINOR_MAX)
			goto erase;

		item = live_minors[entry_minor];
		if (item == NULL) {
			if (entry_minor == CRONYX_CONTROL_DEVICE && entry_major == CRONYX_MJR_BINDER
			    && strcmp (entry->d_name, "binder") == 0)
				continue;
			goto erase;
		}

		switch (entry_major) {
			default:
				goto erase;
			case CRONYX_MJR_BINDER:
				if (item->svc != CRONYX_SVC_DIRECT)
					goto erase;
				break;

			case CRONYX_MJR_ASYNC_SERIAL:
			case CRONYX_MJR_ASYNC_CALLOUT:
				if (item->svc != CRONYX_SVC_TTY_ASYNC)
					goto erase;
				break;

			case CRONYX_MJR_SYNC_SERIAL:
				if (item->svc != CRONYX_SVC_TTY_SYNC)
					goto erase;
				break;
		}

		if (strcmp (item->name, entry->d_name) == 0) {
			bit_set ((entry_major == CRONYX_MJR_ASYNC_CALLOUT)
				? bitmap_done_names_callout : bitmap_done_names, entry_minor);
			continue;
		}

		if (strcmp (item->alias, entry->d_name) == 0) {
			bit_set ((entry_major == CRONYX_MJR_ASYNC_CALLOUT)
				? bitmap_done_aliases_callout : bitmap_done_aliases, entry_minor);
			continue;
		}

erase:
		if (unlink (entry_path)) {
			if (can_ignore_errors)
				continue;
			failed (entry_path);
		}
	}
	closedir (hive);

	for (entry_minor = 0; entry_minor < CRONYX_MINOR_MAX; entry_minor++) {
		item = live_minors[entry_minor];
		if (item == NULL)
			continue;

		if (!bit_test (bitmap_done_names, entry_minor) && item->name[0]) {
			snprintf (entry_path, sizeof (entry_path), "%s/%s", devnode_hive, item->name);
			if (mknod (entry_path, S_IFCHR | 0640, makedev (svc2major (item), entry_minor))) {
				if (can_ignore_errors)
					continue;
				failed (entry_path);
			}
		}

		if (!bit_test (bitmap_done_aliases, entry_minor) && item->alias[0]) {
			snprintf (entry_path, sizeof (entry_path), "%s/%s", devnode_hive, item->alias);
			if (mknod (entry_path, S_IFCHR | 0640, makedev (svc2major (item), entry_minor))) {
				if (can_ignore_errors)
					continue;
				failed (entry_path);
			}
		}

		if (item->svc == CRONYX_SVC_TTY_ASYNC) {
			if (!bit_test (bitmap_done_names_callout, entry_minor) && item->name[0]) {
				snprintf (entry_path, sizeof (entry_path), "%s/%s_cu", devnode_hive, item->name);
				if (mknod (entry_path, S_IFCHR | 0640, makedev (CRONYX_MJR_ASYNC_CALLOUT, entry_minor))) {
					if (can_ignore_errors)
						continue;
					failed (entry_path);
				}
			}

			if (!bit_test (bitmap_done_aliases_callout, entry_minor) && item->alias[0]) {
				snprintf (entry_path, sizeof (entry_path), "%s/%s_cu", devnode_hive, item->alias);
				if (mknod (entry_path, S_IFCHR | 0640, makedev (CRONYX_MJR_ASYNC_CALLOUT, entry_minor))) {
					if (can_ignore_errors)
						continue;
					failed (entry_path);
				}
			}
		}
	}

	if (cronyx_ioctl (binder_fd, CRONYX_PUSH_EVO, &binder_evolution, 0) < 0)
		failed ("binder.push-evolution");
}

int cronyx_set (int target_id, int param_id, ...)
{
	va_list ap;
	struct cronyx_ctl_t ctl;
	int error;

	va_start (ap, param_id);
	ctl.target_id = target_id;
	ctl.param_id = param_id & ~cronyx_flag_canfail;

	switch (ctl.param_id & ~cronyx_flag_channel2link) {
		case cronyx_proto:
			memset (&ctl.u.proto, 0, sizeof (ctl.u.proto));
			strncpy (ctl.u.proto.proname, va_arg (ap, char *), sizeof (ctl.u.proto.proname));
			break;

		case cronyx_dxc:
			memcpy (&ctl.u.dxc, va_arg (ap, char *), sizeof (ctl.u.dxc));
			break;

		case cronyx_clear_stat:
			break;

		case cronyx_led_mode:
			ctl.u.param.value = va_arg (ap, int);
			ctl.u.param.extra = va_arg (ap, long);
			break;

		default:
			ctl.u.param.value = va_arg (ap, long);
			break;
	}

	va_end (ap);
	error = cronyx_ioctl (binder_fd, CRONYX_SET, &ctl, param_id & cronyx_flag_canfail);
	return error;
}

int cronyx_get (int target_id, int param_id, ...)
{
	va_list ap;
	struct cronyx_ctl_t ctl;
	int error;

	ctl.target_id = target_id;
	ctl.param_id = param_id;
	error = cronyx_ioctl (binder_fd, CRONYX_GET, &ctl, silently_failed_get_request);
	if (error >= 0) {
		va_start (ap, param_id);
		switch (param_id & ~cronyx_flag_channel2link) {
			case cronyx_proto:
				memcpy (va_arg (ap, char *), &ctl.u.proto, sizeof (ctl.u.proto));
				break;

			case cronyx_dxc:
				memcpy (va_arg (ap, char *), &ctl.u.dxc, sizeof (ctl.u.dxc));
				break;

			case cronyx_stat_channel:
				memcpy (va_arg (ap, void *), &ctl.u.stat_channel, sizeof (ctl.u.stat_channel));
				break;

			case cronyx_stat_e1:
				memcpy (va_arg (ap, void *), &ctl.u.stat_e1, sizeof (ctl.u.stat_e1));
				break;

			case cronyx_stat_e3:
				memcpy (va_arg (ap, void *), &ctl.u.stat_e3, sizeof (ctl.u.stat_e3));
				break;

			case cronyx_led_mode:
				*va_arg (ap, int *) = ctl.u.param.value;
				*va_arg (ap, long *) = ctl.u.param.extra;
				break;

			default:
				*va_arg (ap, long *) = ctl.u.param.value;
				break;
		}
		va_end (ap);
	}
	return error;
}

void usage (void)
{
	printf ("Cronyx Configuration Utility, version 6.1\n"
		"Copyright (C) 1998-2009 Cronyx Engineering.\n"
		"Usage:\n"
		"  sconfig [-raimsxeftucqv] [<object> [parameters...]]\n"
		"\n"
		"Options:\n"
		"   <none>   -- print <object> options;\n"
		"   -r       -- print current roadmap of objects;\n"
		"   -a       -- print all settings of the <object>;\n"
		"   -i       -- print network interface status;\n"
		"   -m       -- print modem signal status;\n"
		"   -s       -- print channel statistics;\n"
		"   -x       -- print extended statistics;\n"
		"   -e       -- print short E1/G703 statistics;\n"
		"   -f       -- print full E1/G703 statistics;\n"
		"   -t       -- print short E3/T3/STS-1 statistics;\n"
		"   -u       -- print full E3/T3/STS-1 statistics;\n"
		"   -c       -- clear statistics;\n"
		"   -q       -- be quiet;\n"
		"   -v       -- print version information;\n"
		"\nProtocol parameters:\n"
		"  idle      -- no protocol (detach any ones);\n"
		"  sync      -- synchronous protocol;\n"
		"  cisco     -- Cisco/HDLC protocol (network);\n"
		"  fr        -- Frame Relay protocol (network);\n"
		"  dlci=<#>  -- add new DLCI (for Frame Relay only);\n"
		"  rbrg      -- remote 'ethernet' bridge (network);\n"
		"  raw       -- raw direct protocol (for user applications);\n"
		"  zaptel    -- zaptel protocol for asterisk.org;\n"
		"  qlen-limit=<#> -- set auto-grow limit for qlen (DAHDI/zaptel only);\n"
		"  ec-delay=<#>   -- set echo-feedback delay in ms (DAHDI/zaptel only);\n"
		"  packet    -- packetized direct protocol (for user applications);\n"
		"  async     -- asynchronous protocol;\n"
		"\nAdapter parameters:\n"
		"  adapter=...   -- select adapter topology:\n"
		"        separate - separeted channels/interfaces;\n"
		"           split - split E1 interface;\n"
		"             mux - E1 multiplexor (required common clock);\n"
		"          b-mode - legacy for Tau-ISA/E1;\n"
		"  led=..[,..]   -- select adapter's led indication mode,\n"
		"                   modes can be combined with \",\":\n"
		"           smart - led blinking with cadence depends of the\n"
		"                   interface(s) status, this is default;\n"
		"             off - led is off while no events;\n"
		"              on - led is on while no events;\n"
		"         [0x]<#> - set custom 32-bit-field blinking cadence;\n"
		"              rx - flash on each data-receive event;\n"
		"              tx - flash on each data-transmit event;\n"
		"             irq - flash on each hardware-interrupt;\n"
		"             err - flash on errors;\n"
		"\nInterface parameters:\n"
		" loop=...       -- loop/mirror mode:\n"
		"             off - normal opearion (no any loop/mirrot);\n"
		"        internal - internal loopback (tx=>rx);\n"
		"          mirror - mirror back to the line (rx=>tx);\n"
		"          remote - request remote loopback;\n"
		"  dpll={on,off} -- enable/disable 'Digital Phase Lock Loop';\n"
		"  line=...      -- line encoding/modulation:\n"
		"             nrz - NRZ line code (only for serial interfaces);\n"
		"            nrzi - NRZI line code (only for serial interfaces);\n"
		"            hdb3 - HDB3 line code (only for E1/G.703 interfaces);\n"
		"             ami - AMI line code (only for E1/G.703 interfaces);\n"
		"  invclk=...    -- rx/tx clock inversion (for serial interfaces):\n"
		"             off - normal clock mode;\n"
		"         rx-only - inverse only RX-clock logic;\n"
		"         tx-only - inverse only TX-clock logic;\n"
		"            both - inverse both RX and TX clock;\n"
		"  cas=...       -- CAS mode (E1 timeslot 16 signaling):\n"
		"	      off - no CAS, use timeslot 16 for data;\n"
		"             set - insert on TX, check on RX;\n"
		"            pass - pass on TX, check on RX;\n"
		"           cross - engage CAS cross-connector (Tau-PCI/32 only);\n"
		"  clock=...     -- E1/G.703 transmit clock selection:\n"
		"        self,int - use internal clock generator;\n"
		"         receive - use received clock;\n"
		"          rcv<#> - use received clock from interface #<num>;\n"
		"  crc4={on,off} -- enable/disable E1 CRC4-multiframing;\n"
		"  unframed={on,off}  -- select framed/unframed E1 mode;\n"
		"  higain={on,off}    -- E1 high non linear input sensitivity\n"
		"                        (e.g. for long line);\n"
		"  monitor={on,off}   -- E1 high linear input sensitivity\n"
		"                        (e.g. interception mode);\n"
		"  scrambler={on,off} -- G.703 scrambling mode;\n"
		"  t3-long={on,off}   -- T3/STS-1 high transmitter output for long cable;\n"
		"\nChannel parameters:\n"
		"  <number>      -- baud rate, internal clock;\n"
		"  extclock      -- external clock (default);\n"
		"   mode=...     -- channel payload format:\n"
		"           async - asynchronous data (for Sigma-ISA only);\n"
		"            hdlc - synchronous mode with HDLC Layer 2 frames;\n"
		"           phony - raw phony-like bytes;\n"
		"           voice - raw phony-like bytes with voice;\n"
		"  ts=...        -- select E1 timeslots;\n"
		"  subchan=...   -- E1 subchannel timeslots (for Tau-ISA only);\n"
		"  iface=<#>     -- bind channel to interface#<num>;\n"
		"  mtu={size}    -- set MTU in bytes;\n"
		"  qlen={len}    -- set transmit/receive queues length in packets/chunks;\n"
		"  crc={mode}    -- select FCS (CRC) mode for HDLC:\n"
		"            none - no FCS generation/checking;\n"
		"              16 - use 16-bit FCS (default mode);\n"
		"              32 - use 32-bit FCS;\n"
		"  sflg={on,off} -- turn on/off share flag transmission for HDLC:\n"
		"             off - transmit both frame-end and frame-begin flags;\n"
		"              on - share frame-end and frame-begin flags, transmit one;\n"
		"\nCommon parameters:\n"
		"  debug={0,1,2} -- enable/disable debug messages;\n" "\nSee also man sconfig (8)\n");
	exit (0);
}

const char *lookup_ioctl_name (long code)
{
#define _(x) if (x == code) return #x
	_(CRONYX_BUNDLE_VER);
	_(CRONYX_ITEM_ENUM);
	_(CRONYX_ITEM_INFO);
	_(CRONYX_SELF_INFO);
	_(CRONYX_GET);
	_(CRONYX_SET);
	_(CRONYX_PUSH_EVO);
#ifdef CRONYX_LYSAP
	_(LYSAP_PEER_ADD);
	_(LYSAP_PEER_REMOVE);
	_(LYSAP_TRUNK_ADD);
	_(LYSAP_TRUNK_REMOVE);
	_(LYSAP_TRUNK_INFO);
	_(LYSAP_FIBERS_INFO);
	_(LYSAP_CPU_INFO);
#endif /* CRONYX_LYSAP */
	else {
		static char buffer[32];

		sprintf (buffer, "0x%05lX", code);
		return buffer;
	}
#undef _
}

const char *cronyx_param_name (int param_id)
{
#define _(x) if (cronyx_##x == param_id)	return #x
	_(channel_mode);
	_(loop_mode);
	_(line_code);
	_(adapter_mode);
	_(cas_mode);
	_(invclk_mode);
	_(led_mode);
	_(port_or_cable_type);
	_(sync_mode);
	_(timeslots_use);
	_(timeslots_subchan);
	_(baud);
	_(iface_bind);
	_(debug);
	_(add_dlci);
	_(modem_status);
	_(crc4);
	_(dpll);
	_(higain);
	_(master);
	_(unframed);
	_(monitor);
	_(scrambler);
	_(t3_long);
	_(inlevel_sdb);
	_(mtu);
	_(qlen);
	_(proto);
	_(dxc);
	_(stat_channel);
	_(stat_e1);
	_(stat_e3);
	_(reset);
	_(clear_stat);
#undef _
	return "unknown-param";
}

int cronyx_ioctl (int fd, long code, void *arg, int canfail)
{
	int error;

	error = ioctl (fd, code, arg);
	if (error < 0 && ! canfail) {
		const char *a, *b;
		if (code == CRONYX_GET) {
			a = "get"; b = cronyx_param_name (((struct cronyx_ctl_t *) arg)->param_id);
		} else if (code == CRONYX_SET) {
			a = "set"; b = cronyx_param_name (((struct cronyx_ctl_t *) arg)->param_id);
		} else {
			a = "ioctl"; b = lookup_ioctl_name (code);
		}
		fprintf (stderr, "sconfig.%s.%s: %s\n", a, b, strerror (errno));
		global_error_code = error;
	}
	return error;
}

u32 scan_timeslots (char *s, char **end)
{
	char *e;
	unsigned v, l;
	u32 ts;

	if (! end) {
		e = strchr (s, '=');
		if (e)
			s = e + 1;
	}

	ts = 0; l = v = 33;
	for (;;) {
		switch (*s) {
			case 0:
			case ' ':
				if (l != 32)
					goto format;
				return ts;
			case '-':
				if (v > 31)
					goto format;
				l = v;
				v = 32;
				s++;
				continue;
			case ',':
				if (l != 32)
					goto format;
				s++;
			default:
				if (*s >= '0' && *s <= '9') {
					v = 0; do {
						v = v * 10 + *s - '0';
						if (v > 31) {
							if (end)
								return 0;
							failed ("invalid value in timeslot-list");
						}
						s++;
					} while (*s >= '0' && *s <= '9');

					ts |= 1ul << v;
					if (l < 32) {
						if (l > v)
							goto format;
						for (; l < v; ++l)
							ts |= 1ul << l;
						v = 32;
					}
					l = 32;

					if (end)
						*end = s;
					continue;
				} else {
					if (end && l == 32)
						return ts;
					goto format;
				}
		}
	}

format:
	if (end)
		return 0;
	failed ("invalid format of timeslot-list");
	return 0;
}

char *format_timeslots (u32 s)
{
	static char buf[100];
	char *p = buf;
	int i;

	for (i = 0; i < 32; ++i) {
		if ((s >> i) & 1) {
			int prev = (i > 0) & (s >> (i - 1));
			int next = (i < 31) & (s >> (i + 1));

			if (prev) {
				if (next)
					continue;
				*p++ = '-';
			} else if (p > buf)
				*p++ = ',';

			if (i >= 10)
				*p++ = '0' + i / 10;
			*p++ = '0' + i % 10;
		}
	}
	*p = 0;
	return buf;
}

void print_modems (struct cronyx_item_info_t *item)
{
	long status = -1;
	int fd = -1;

#if 1
	if (cronyx_get (item->id, cronyx_modem_status, &status) < 0) {
		if (!silently_failed_get_request)
			perror ("binder.get_modem_status");
		return;
	}
#else
	char buffer[PATH_MAX + 1];

	if (item->minor > 0) {
		if (item->name[0]) {
			snprintf (buffer, sizeof (buffer), "%s/%s%s", devnode_hive, item->name,
				  (item->svc == CRONYX_SVC_TTY_ASYNC) ? "_cu" : "");
			fd = open (buffer, 0);
		}
		if (fd < 0 && item->alias[0]) {
			snprintf (buffer, sizeof (buffer), "%s/%s%s", devnode_hive, item->alias,
				  (item->svc == CRONYX_SVC_TTY_ASYNC) ? "_cu" : "");
			fd = open (buffer, 0);
		}
	}

	if (fd >= 0) {
		if (cronyx_ioctl (fd, TIOCMGET, &status, silently_failed_get_request) < 0) {
			if (!silently_failed_get_request)
				perror ("devnode.get_modem_status");
			return;
		}
	} else {
		if (cronyx_get (item->id, cronyx_modem_status, &status) < 0) {
			if (!silently_failed_get_request)
				perror ("binder.get_modem_status");
			return;
		}
	}
#endif

	if ((header_flags & SCONFIG_HEADER_MODEM) == 0) {
		header_flags |= SCONFIG_HEADER_MODEM;
		printf ("Interface\tLE\tDTR\tDSR\tRTS\tCTS\tCD\n");
	}
	print_item_name (item, 9);
	printf ("\t%s\t%s\t%s\t%s\t%s\t%s\n",
		status & TIOCM_LE ? "On" : "-",
		status & TIOCM_DTR ? "On" : "-",
		status & TIOCM_DSR ? "On" : "-",
		status & TIOCM_RTS ? "On" : "-", status & TIOCM_CTS ? "On" : "-", status & TIOCM_CD ? "On" : "-");

	if (fd >= 0)
		close (fd);
}

void print_huge (u64 val)
{
	const char *s = ".KMGTPEZY"; /* LY: Kilo, Mega, Giga, Tera, Peta, Exa, Zetta, Yotta! */
	char buf[16];
	int l;
	double k = 1.0;

	strcpy (buf, "-");
	if (val) {
		do {
			if (*s == '.')
				l = snprintf (buf, sizeof(buf), "%'llu", (unsigned long long) val);
			else {
				k *= 1000.0;
				l = snprintf (buf, sizeof(buf), "%'.1f%c", (double) val / k, *s);
			}
		} while (l > 7 && *++s);
	}
	printf (" %7s", buf);
}

void print_item_name (struct cronyx_item_info_t *item, int width)
{
	width -= printf ("%s%s%s", item->name, item->alias[0] ? "/" : "", item->alias);
	while (width-- > 0)
		putchar (' ');
}

void print_stats_rs (struct cronyx_item_info_t *item)
{
	struct cronyx_serial_statistics st;

	if (cronyx_get (item->id, cronyx_stat_channel, &st) < 0) {
		if (!silently_failed_get_request) {
		print_item_name (item, 16);
			perror ("binder.get_stat_channel");
		}
		return;
	}

	if ((header_flags & SCONFIG_HEADER_STATCHAN) == 0) {
		header_flags |= SCONFIG_HEADER_STATCHAN;
		if (sflag) {
			printf ("                 Receive_______________] Transmit______________]\n");
			printf ("Channel________] __Intrs Packets _Errors __Intrs Packets _Errors\n");
		} else {
			printf ("                 Receive_______________________] Transmit______________________] Modem__\n");
			printf ("Channel________] __Intrs __Bytes Packets _Errors __Intrs __Bytes Packets _Errors __Intrs\n");
		}
	}
	print_item_name (item, 16);

	print_huge (st.rintr);
	if (!sflag)
		print_huge (st.ibytes);
	print_huge (st.ipkts);
	print_huge (st.ierrs);

	print_huge (st.tintr);
	if (!sflag)
		print_huge (st.obytes);
	print_huge (st.opkts);
	print_huge (st.oerrs);

	if (!sflag)
		print_huge (st.mintr);
	printf ("\n");
}

void clear_stats (int id)
{
	if (cronyx_set (id, cronyx_clear_stat) < 0) {
		failed ("binder.stat_clear");
	}
}

const char *format_e1_status (u32 status, int loop_mode)
{
	static char buf[64];

	if (status == CRONYX_E1_PENDING)
		return "n/a";
	if (status == CRONYX_E1_OFF)
		return "OFF";

	buf[0] = 0;
	switch (loop_mode) {
		case CRONYX_LOOP_INTERNAL:
			status &= ~CRONYX_E1_LOS;
			strcat (buf, ",LOOP");
			break;
		case CRONYX_LOOP_LINEMIRROR:
			strcat (buf, ",MIRROR");
			break;
		case CRONYX_LOOP_REMOTE:
		case CRONYX_LOOP_NONE:
			break;
	}
	if (status == 0)
		strcat (buf, ",OK");
	if (status & CRONYX_E3_TXE)
		strcat (buf, ",XMIT-ERR");
	if (status & CRONYX_E1_LOS)
		strcat (buf, ",LOS");
	else if (status & CRONYX_E1_AIS)
		strcat (buf, ",AIS");
	else if (status & CRONYX_E1_LOF)
		strcat (buf, ",LOF");
	else {
		if (status & CRONYX_E1_CRC4E)
			strcat (buf, ",CRC4E");
		if (status & CRONYX_E1_RA)
			strcat (buf, ",RA");
		if (status & CRONYX_E1_AIS16)
			strcat (buf, ",AIS16");
		else if (status & CRONYX_E1_LOMF)
			strcat (buf, ",LOMF");
		else if (status & CRONYX_E1_CASERR)
			strcat (buf, ",CAS-ERROR");
		else if (status & CRONYX_E1_RDMA)
			strcat (buf, ",RDMA");
	}
#ifdef CRONYX_E1_TSTREQ
	if (status & CRONYX_E1_TSTREQ)
		strcat (buf, ",TSTREQ");
#endif
#ifdef CRONYX_E1_TSTERR
	if (status & CRONYX_E1_TSTERR)
		strcat (buf, ",TSTERR");
#endif
	if (buf[0])
		return buf + 1;
	return "Unknown";
}

void print_frac (u32 numerator, u32 divider)
{
#if 1
	const char *s = 0;

	if (numerator < 1)
		s = "-";
	else if (numerator == divider)
		s = "all";
	else if (divider < 1) {
		print_huge (numerator);
		return;
	} else {
		int e = 0;
		double v = (double) numerator / divider;

		while (v > 9.99) {
			e++;
			v /= 10.0;
		}
		while (v < 0.01) {
			e--;
			v *= 10.0;
		}
		if (e == 0)
			printf (" %7.5f", v);
		else if (e > -10 && e < 10)
			printf (" %5.3f%+d", v, e);
		else
			printf (" %4.2f%+03d", v, e);
		return;
	}
	printf (" %7s", s);
#else
	char buf[64];
	sprintf (buf, "%u/%u", numerator, divider);
	printf (" %7s", buf);
#endif
}

void print_e1_stats (struct cronyx_item_info_t *item)
{
	struct cronyx_e1_statistics st;
	int i, maxi;
	struct getset_param_t loop_mode;

	if (cronyx_get (item->id, cronyx_stat_e1, &st) < 0) {
		if (!silently_failed_get_request) {
			print_item_name (item, 16);
			perror ("binder.get_stat_e1");
		}
		return;
	}

	if ((header_flags & SCONFIG_HEADER_STATE1) == 0) {
		header_flags |= SCONFIG_HEADER_STATE1;
		printf ("Interface______] [___UAS [____DM [___BPV [___OOF [__CRC4 [__RCRC [____ES [___LES [___SES [___BES [__SEFS [___CSS Status\n");
	}
	print_item_name (item, 16);

	/*
	 * Unavailable seconds, degraded minutes
	 */
	print_frac (st.currnt.uas, st.cursec);
	print_frac (60 * st.currnt.dm, st.cursec);

	/*
	 * Bipolar violations, frame sync errors
	 */
	print_frac (st.currnt.bpv, st.cursec);
	print_frac (st.currnt.fse, st.cursec);

	/*
	 * CRC errors, remote CRC errors (E-bit)
	 */
	print_frac (st.currnt.crce, st.cursec);
	print_frac (st.currnt.rcrce, st.cursec);

	/*
	 * Errored seconds, line errored seconds
	 */
	print_frac (st.currnt.es, st.cursec);
	print_frac (st.currnt.les, st.cursec);

	/*
	 * Severely errored seconds, bursty errored seconds
	 */
	print_frac (st.currnt.ses, st.cursec);
	print_frac (st.currnt.bes, st.cursec);

	/*
	 * Out of frame seconds, controlled slip seconds
	 */
	print_frac (st.currnt.oofs, st.cursec);
	print_frac (st.currnt.css, st.cursec);

	cronyx_get_param (item->id, &loop_mode, cronyx_loop_mode);
	printf (" %s\n", format_e1_status (st.status, loop_mode.valid ? loop_mode.valid : CRONYX_LOOP_NONE));

	if (fflag) {
		/*
		 * Print total statistics.
		 */
		printf ("           total");
		print_frac (st.total.uas, st.totsec);
		print_frac (60 * st.total.dm, st.totsec);

		print_frac (st.total.bpv, st.totsec);
		print_frac (st.total.fse, st.totsec);

		print_frac (st.total.crce, st.totsec);
		print_frac (st.total.rcrce, st.totsec);

		print_frac (st.total.es, st.totsec);
		print_frac (st.total.les, st.totsec);

		print_frac (st.total.ses, st.totsec);
		print_frac (st.total.bes, st.totsec);

		print_frac (st.total.oofs, st.totsec);
		print_frac (st.total.css, st.totsec);

		printf ("\n");

		/*
		 * Print 24-hour history.
		 */
		maxi = (st.totsec - st.cursec) / 900;
		if (maxi > 48)
			maxi = 48;
		for (i = 0; i < maxi; ++i) {
			if (i < 3)
				printf ("%15um", (i + 1) * 15);
			else
				printf ("%11dh %2dm", (i + 1) / 4, (i + 1) % 4 * 15);
			print_frac (st.interval[i].uas, 15 * 60);
			print_frac (st.interval[i].dm, 0);

			print_frac (st.interval[i].bpv, 15 * 60);
			print_frac (st.interval[i].fse, 15 * 60);

			print_frac (st.interval[i].crce, 15 * 60);
			print_frac (st.interval[i].rcrce, 15 * 60);

			print_frac (st.interval[i].es, 15 * 60);
			print_frac (st.interval[i].les, 15 * 60);

			print_frac (st.interval[i].ses, 15 * 60);
			print_frac (st.interval[i].bes, 15 * 60);

			print_frac (st.interval[i].oofs, 15 * 60);
			print_frac (st.interval[i].css, 15 * 60);

			printf ("\n");
		}
	}
}

char *format_e3_cv (u32 cv, u32 baud, u32 time)
{
	static char buf[64];

	if (!cv || !baud || !time)
		snprintf (buf, sizeof(buf), "     -     ");
	else
		snprintf (buf, sizeof(buf), "%10lu (%.1e)", (unsigned long) cv, (double) cv / baud / time);
	return buf;
}

void print_e3_stats (struct cronyx_item_info_t *item)
{
	struct cronyx_e3_statistics st;
	struct getset_param_t loop_mode;
	int i, maxi;
	u32 baud;

	if (cronyx_get (item->id, cronyx_stat_e3, &st) < 0) {
		if (!silently_failed_get_request) {
			print_item_name (item, 16);
			perror ("binder.get_stat_e3");
		}
		return;
	}

	if (cronyx_get (item->id, cronyx_baud, &baud) < 0) {
		if (!silently_failed_get_request) {
			print_item_name (item, 16);
			perror ("binder.get_baud");
		}
		return;
	}

	if ((header_flags & SCONFIG_HEADER_STATE3) == 0) {
		header_flags |= SCONFIG_HEADER_STATE3;
		printf ("Interface\t--Code Violations---\t\t\t\t\t ----Status----\n");
	}

	if (!st.cursec)
		st.cursec = 1;

	print_item_name (item, 16);
	printf ("\t%s\t\t\t\t\t", format_e3_cv (st.ccv, baud, st.cursec));

	cronyx_get_param (item->id, &loop_mode, cronyx_loop_mode);
	printf (" %s\n", format_e1_status (st.status, loop_mode.valid ? loop_mode.valid : CRONYX_LOOP_NONE));

	if (uflag) {
		/*
		 * Print total statistics.
		 */
		printf ("\t%s\t\t\t\t\t", format_e3_cv (st.tcv, baud, st.totsec));
		printf (" -- Total\n");

		/*
		 * Print 24-hour history.
		 */
		maxi = (st.totsec - st.cursec) / 900;
		if (maxi > 48)
			maxi = 48;
		for (i = 0; i < maxi; ++i) {
			printf ("\t%s\t\t\t\t\t", format_e3_cv (st.icv[i], baud, 15 * 60));
			if (i < 3)
				printf (" -- %2dm\n", (i + 1) * 15);
			else
				printf (" -- %2dh %2dm\n", (i + 1) / 4, (i + 1) % 4 * 15);
		}
	}
}

void failed (const char *reason)
{
	if (reason) {
		if (errno)
			perror (reason);
		else
			fprintf (stderr, "sconfig: %s\n", reason);
	}

	fprintf (stderr, "sconfig: failed%s\n", isatty (fileno (stderr)) ? "\7" : "");
	exit (-1);
}

int get_switch (char *line, const char *variants, ...)
{
	char *value;
	const char *key;
	int order, result;
	va_list ap;

	value = strchr (line, '=');
	if (value == NULL || *value == 0) {
		fprintf (stderr, "sconfig: value must be specified, e.g. '%s=value'\n", line);
		failed ("invalid command line");
	}

	key = variants;
	order = 0;
	do {
		int len;
		char *alt = strchr (key, ',');

		if (alt)
			len = alt - key;
		else
			len = strlen (key);
		if (strncasecmp (value + 1, key, len) == 0 && value[len + 1] == 0) {
			va_start (ap, variants);
			do
				result = va_arg (ap, int);
			while (--order >= 0);
			va_end (ap);
			return result;
		}
		if (alt == 0)
			order++;
		key += len + 1;
	} while (*key);

	*value++ = 0;
	fprintf (stderr, "sconfig: unknown value '%s' is specified for switch '%s'\n", value, line);
	fprintf (stderr, "sconfig: available options for '%s' are: ", line);

	key = variants;
	do {
		fprintf (stderr, "%s%s", (key == variants) ? "" : ", ", key);
		key += strlen (key) + 1;
	} while (*key);
	fprintf (stderr, "\n");

	failed ("invalid command line");
	return -1;
}

int get_ledmode (char *line, u32 *cadence)
{
	unsigned char ledmode = CRONYX_LEDMODE_DEFAULT;
	char *scan = strchr (line, '=');

	if (scan == NULL || *scan == 0) {
		fprintf (stderr, "sconfig: value must be specified, e.g. '%s=value'\n", line);
		failed ("invalid command line");
	}
	while (scan && *++scan) {
		static const char *options[] = {
			"smart", "rx", "tx", "error", "irq", "on", "off", NULL
		};
		static const unsigned char flags[] = {
			CRONYX_LEDMODE_DEFAULT, CRONYX_LEDMODE_4RX, CRONYX_LEDMODE_4TX,
			CRONYX_LEDMODE_4ERR, CRONYX_LEDMODE_4IRQ, CRONYX_LEDMODE_ON,
			CRONYX_LEDMODE_OFF
		};
		int r, j;
		char *comma = strchr (scan, ',');

		for (j = 0; options[j] != NULL; j++) {
			if (comma == NULL) {
				r = strcasecmp (scan, options[j]);
			} else {
				r = strncasecmp (scan, options[j], comma - scan);
				if (r == 0)
					r = (comma - scan) - strlen (options[j]);
			}
			if (r == 0)
				break;
		}
		if (options[j]) {
			ledmode = (ledmode & ~CRONYX_LEDMODE_CADENCE) | flags[j];
			if (flags[j] == CRONYX_LEDMODE_DEFAULT)
				ledmode = CRONYX_LEDMODE_DEFAULT;
		} else {
			char *end = NULL;
			unsigned long may_cadence = strtoul (scan, &end, 10);

			if (*end != 0 && end != comma && strncasecmp (scan, "0x", 2) == 0)
				may_cadence = strtoul (scan + 2, &end, 16);
			if (*end != 0 && end != comma && strncasecmp (scan, "O", 1) == 0)
				may_cadence = strtoul (scan + 1, &end, 8);
			if (*end != 0 && end != comma) {
				fprintf (stderr, "sconfig: invalid led syntax near '%s'\n", scan);
				failed ("invalid command line");
			}
			*cadence = may_cadence;
			ledmode |= CRONYX_LEDMODE_CADENCE;
		}
		scan = comma;
	}
	return ledmode;
}

int is_cmd_param (const char *param, const char *arg)
{
	int len = strlen (param);
	int result = strncasecmp (arg, param, len) == 0 && arg[len] == '=';

	return result;
}

int get_bool_switch (char *line)
{
	return get_switch (line, "on,yes,true,1\0off,no,false,0\0", 1, 0);
}

void list_items ()
{
	int i;
	struct cronyx_item_info_t *item = binder_roadmap;

	for (i = 0; i < roadmap_items; i++, item++)
		if (item->type == list_mode)
			printf ("%s\n", item->alias[0] ? item->alias : item->name);
}

int print_roadmap (int parent, int level)
{
	int i, j, result;
	struct cronyx_item_info_t *item;

	result = 0;
	item = binder_roadmap;
	for (i = 0; i < roadmap_items; i++, item++) {
		if (item->parent == parent && item->type > CRONYX_ITEM_REMOVED) {
			char *type;

			for (j = 0; j < level; j++)
				putchar ('\t');
			print_item_name (item, 9);
			switch (item->type) {
				case CRONYX_ITEM_ADAPTER:
					type = "adapter";
					break;
				case CRONYX_ITEM_INTERFACE:
					type = "interface";
					break;
				case CRONYX_ITEM_CHANNEL:
					type = "channel";
					break;
				default:
					type = "unknown";
			}
			printf (" (%s", type);
			if (item->order >= 0)
				printf (" #%d", item->order + 1);
			printf (")\n");

			j = print_roadmap (item->id, level + 1);
			if (j > 0) {
				result += j;
				putchar ('\n');
			}
			result++;
		}
	}
	return result;
}

void cronyx_get_param (int id, struct getset_param_t *param, int param_id)
{
	param->valid = cronyx_get (id, param_id, &param->value, &param->extra) >= 0;
}
