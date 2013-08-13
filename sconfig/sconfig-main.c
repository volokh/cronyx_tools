/*
 * Channel configuration utility for Cronyx serial adapters.
 *
 * Copyright (C) 1997-2002 Cronyx Engineering.
 * Author: Serge Vakulenko <vak@cronyx.ru>
 *
 * Copyright (C) 1999-2005 Cronyx Engineering.
 * Author: Roman Kurakin <rik@cronyx.ru>
 *
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
 * $Id: sconfig-main.c,v 1.51 2009-10-08 14:57:12 ly Exp $
 */

#include "sconfig.h"

char vflag, eflag, sflag, mflag, cflag, fflag, iflag, flag_show_all;
char xflag, tflag, uflag, flag_roadmap, silently_failed_get_request = 1;
char need_update_hive, flag_quiet;
char list_mode;

int binder_fd = -1;
int global_error_code;
int header_flags;

int print_ifconfig (struct cronyx_item_info_t *node)
{
	char buffer[64];
	struct cronyx_proto_t proto;

	fflush (stdout);
	if (cronyx_get (node->id, cronyx_proto, &proto) >= 0 && proto.proname[0]) {
		if (strcmp (proto.proname, "fr") == 0)
			sprintf (buffer, "ifconfig %sd16 2>/dev/null", node->alias);
		else
			sprintf (buffer, "ifconfig %s 2>/dev/null", node->alias);
	}

	return system (buffer);
}

void print_chan (struct cronyx_item_info_t *node)
{
	struct cronyx_proto_t proto;
	int proto_valid;

	struct getset_param_t adapter_mode, baud, casmode, crc4, debug, dpll, t3_long, higain,
		iface, inlevel_sdb, invclk_mode, line_code, loop_mode, monitor, mtu,
		qlen, scrambler, timeslots_use, timeslots_subchan, unframed, sync_mode, rs_port_or_cable_type,
		channel_mode, led_mode, ec_delay, qlen_limit, crc_mode, hdlc_flags, cas_flags,
		iface_updown;

	cronyx_get_param (node->id, &adapter_mode, cronyx_adapter_mode);
	cronyx_get_param (node->id, &led_mode, cronyx_led_mode);
	cronyx_get_param (node->id, &channel_mode, cronyx_channel_mode);
	cronyx_get_param (node->id, &baud, cronyx_baud);
	cronyx_get_param (node->id, &casmode, cronyx_cas_mode);
	cronyx_get_param (node->id, &crc4, cronyx_crc4);
	cronyx_get_param (node->id, &debug, cronyx_debug);
	cronyx_get_param (node->id, &dpll, cronyx_dpll);
	cronyx_get_param (node->id, &t3_long, cronyx_t3_long);
	cronyx_get_param (node->id, &higain, cronyx_higain);
	cronyx_get_param (node->id, &iface, cronyx_iface_bind);
	cronyx_get_param (node->id, &inlevel_sdb, cronyx_inlevel_sdb);
	cronyx_get_param (node->id, &invclk_mode, cronyx_invclk_mode);
	cronyx_get_param (node->id, &line_code, cronyx_line_code);
	cronyx_get_param (node->id, &loop_mode, cronyx_loop_mode);
	cronyx_get_param (node->id, &monitor, cronyx_monitor);
	cronyx_get_param (node->id, &mtu, cronyx_mtu);
	cronyx_get_param (node->id, &qlen, cronyx_qlen);
	cronyx_get_param (node->id, &scrambler, cronyx_scrambler);
	cronyx_get_param (node->id, &timeslots_use, cronyx_timeslots_use);
	cronyx_get_param (node->id, &timeslots_subchan, cronyx_timeslots_subchan);
	cronyx_get_param (node->id, &unframed, cronyx_unframed);
	cronyx_get_param (node->id, &sync_mode, cronyx_sync_mode);
	cronyx_get_param (node->id, &rs_port_or_cable_type, cronyx_port_or_cable_type);
	cronyx_get_param (node->id, &ec_delay, cronyx_ec_delay);
	cronyx_get_param (node->id, &qlen_limit, cronyx_qlen_limit);
	cronyx_get_param (node->id, &crc_mode, cronyx_crc_mode);
	cronyx_get_param (node->id, &hdlc_flags, cronyx_hdlc_flags);
	cronyx_get_param (node->id, &cas_flags, cronyx_cas_flags);
	cronyx_get_param (node->id, &iface_updown, cronyx_iface_updown);
	proto_valid = cronyx_get (node->id, cronyx_proto, &proto) >= 0;

	print_item_name (node, 12);
	if (rs_port_or_cable_type.valid) {
		switch (rs_port_or_cable_type.value) {
			case CRONYX_NONE:
				printf ("(no cable)");
				break;
			case CRONYX_RS232:
				printf ("(RS-232)");
				break;
			case CRONYX_RS449:
				printf ("(RS-449)");
				break;
			case CRONYX_RS530:
				printf ("(RS-530)");
				break;
			case CRONYX_E1:
				printf ("(E1)");
				break;
			case CRONYX_E3:
				printf ("(E3)");
				break;
			case CRONYX_V35:
				printf ("(V.35)");
				break;
			case CRONYX_G703:
				printf ("(G.703)");
				break;
			case CRONYX_X21:
				printf ("(X.21)");
				break;
			case CRONYX_RS485:
				printf ("(RS-485)");
				break;
			case CRONYX_COAX:
				printf ("(Coax)");
				break;
			case CRONYX_TP:
				printf ("(Twin Pair)");
				break;
			case CRONYX_SERIAL:
				printf ("(Serial)");
				break;
			case CRONYX_AUTO:
				printf ("(Auto Select)");
				break;
			case CRONYX_UNKNOWN:
			default:
				printf ("(?)");
				break;
		}
	}

	if (iface_updown.valid) {
		switch (iface_updown.value) {
			case CRONYX_IFACE_UP:
				if (flag_show_all)
					printf (" %s", "up");
				break;
			case CRONYX_IFACE_DOWN:
				printf (" %s", "down");
				break;
			default:
				printf (" iface_updown=%s", "?");
		}
	}

	if (proto_valid) {
		if (proto.proname[0])
			printf (" %.8s", proto.proname);
		else
			printf (" idle");
	}

	if (debug.valid && debug.value)
		printf (" debug=%ld", debug.value);

	if (crc_mode.valid) {
		switch (crc_mode.value) {
			case CRONYX_CRC_NONE:
				printf (" crc=%s", "none");
				break;
			case CRONYX_CRC_16:
				if (flag_show_all)
					printf (" crc=%s", "16");
				break;
			case CRONYX_CRC_32:
				printf (" crc=%s", "32");
				break;
			default:
				printf (" crc=%s", "?");
		}
	}

	if (hdlc_flags.valid) {
		switch (hdlc_flags.value) {
			case CRONYX_HDLC_2FLAGS:
				if (flag_show_all)
					printf (" sflg=%s", "off");
				break;
			case CRONYX_HDLC_SHARE:
				printf (" sflg=%s", "on");
				break;
			default:
				printf (" sflg=%s", "?");
		}
	}

	if (cas_flags.valid) {
		switch (cas_flags.value) {
			case CRONYX_CAS_ITU:
				if (flag_show_all)
					printf (" cas-strict=%s", "off");
				break;
			case CRONYX_CAS_STRICT:
				printf (" cas-strict=%s", "on");
				break;
			default:
				printf (" cas-options=%s", "?");
		}
	}

	if (t3_long.valid)
		printf (" t3-long=%s", t3_long.value ? "on" : "off");

	if (adapter_mode.valid) {
		printf (" adapter=");
		switch (adapter_mode.value) {
			case CRONYX_MODE_SEPARATE:
				printf ("separate");
				break;
			case CRONYX_MODE_B:
				printf ("b-mode");
				break;
			case CRONYX_MODE_SPLIT:
				printf ("split");
				break;
			case CRONYX_MODE_MUX:
				printf ("mux");
				break;
			default:
				printf ("?%ld", adapter_mode.value);
		}
	}

	if (led_mode.valid) {
		if (led_mode.value != CRONYX_LEDMODE_DEFAULT || flag_show_all) {
			printf (" led=");
			switch (led_mode.value & CRONYX_LEDMODE_CADENCE) {
				case CRONYX_LEDMODE_CADENCE:
					printf ("0x%lX", led_mode.extra);
					break;
				case CRONYX_LEDMODE_ON:
					printf ("on");
					break;
				case CRONYX_LEDMODE_OFF:
					printf ("off");
					break;
				default:
					printf ("smart");
			}
			if (led_mode.value & CRONYX_LEDMODE_4IRQ)
				printf (",irq");
			if (led_mode.value & CRONYX_LEDMODE_4RX)
				printf (",rx");
			if (led_mode.value & CRONYX_LEDMODE_4TX)
				printf (",tx");
			if (led_mode.value & CRONYX_LEDMODE_4ERR)
				printf (",err");
		}
	}

	if (channel_mode.valid) {
		switch (channel_mode.value) {
			case CRONYX_MODE_ASYNC:
				printf (" mode=async");
				break;
			case CRONYX_MODE_HDLC:
				if (flag_show_all)
					printf (" mode=hdlc");
				break;
			case CRONYX_MODE_PHONY:
				printf (" mode=phony");
				break;
			case CRONYX_MODE_VOICE:
				printf (" mode=voice");
				break;
			default:
				printf (" mode=?%ld", channel_mode.value);
		}
	}

	if (iface.valid)
		printf (" iface=%ld", iface.value);

	if (baud.valid) {
		if (baud.value)
			printf (" %ld", baud.value);
		else
			printf (" extclock");
	}

	if (sync_mode.valid) {
		printf (" clock=");
		switch (sync_mode.value) {
			case CRONYX_E1CLK_INTERNAL:
				printf ("internal");
				break;
			case CRONYX_E1CLK_RECEIVE:
				printf ("receive");
				break;
			case CRONYX_E1CLK_RECEIVE_CHAN0:
				printf ("rcv0");
				break;
			case CRONYX_E1CLK_RECEIVE_CHAN1:
				printf ("rcv1");
				break;
			case CRONYX_E1CLK_RECEIVE_CHAN2:
				printf ("rcv2");
				break;
			case CRONYX_E1CLK_RECEIVE_CHAN3:
				printf ("rcv3");
				break;
			default:
				printf ("?%ld", sync_mode.value);
				break;
		}
	}

	if (dpll.valid)
		printf (" dpll=%s", dpll.value ? "on" : "off");

	if (invclk_mode.valid) {
		switch (invclk_mode.value) {
			default:
				printf (" invclk=?%ld", invclk_mode.value);
				break;
			case CRONYX_ICLK_NORMAL:
				if (flag_show_all)
					printf (" invclk=%s", "none");
				break;
			case CRONYX_ICLK_RX:
				printf (" invclk=%s", "rx-only");
				break;
			case CRONYX_ICLK_TX:
				printf (" invclk=%s", "tx-only");
				break;
			case CRONYX_ICLK:
				printf (" invclk=%s", "both");
				break;
		}
	}

	if (unframed.valid)
		printf (" unframed=%s", unframed.value ? "on" : "off");

	if (casmode.valid) {
		printf (" cas=");
		switch (casmode.value) {
			default:
				printf ("?%ld", casmode.value);
				break;
			case CRONYX_CASMODE_OFF:
				printf ("off");
				break;
			case CRONYX_CASMODE_SET:
				printf ("set");
				break;
			case CRONYX_CASMODE_PASS:
				printf ("pass");
				break;
			case CRONYX_CASMODE_CROSS:
				printf ("cross");
				break;
		}
	}

	if (loop_mode.valid) {
		switch (loop_mode.value) {
			case CRONYX_LOOP_NONE:
				if (flag_show_all)
					printf (" loop=%s", "none");
				break;
			case CRONYX_LOOP_INTERNAL:
				printf (" loop=%s", "internal");
				break;
			case CRONYX_LOOP_LINEMIRROR:
				printf (" loop=%s", "mirror");
				break;
			case CRONYX_LOOP_REMOTE:
				printf (" loop=%s", "remote");
				break;
			default:
				printf (" loop=?%ld", loop_mode.value);
				break;
		}
	}

	if (line_code.valid) {
		switch (line_code.value) {
			case CRONYX_NRZ:
				if (flag_show_all)
					printf (" line=%s", "nrz");
				break;
			case CRONYX_NRZI:
				printf (" line=%s", "nrzi");
				break;
			case CRONYX_HDB3:
				if (flag_show_all)
					printf (" line=%s", "hdb3");
				break;
			case CRONYX_AMI:
				printf (" line=%s", "ami");
				break;
			default:
				printf (" line=?%ld", loop_mode.value);
				break;
		}
	}

	if (flag_show_all) {
		if (mtu.valid)
			printf (" mtu=%ld", mtu.value);
		if (qlen.valid)
			printf (" qlen=%ld", qlen.value);
		if (qlen_limit.valid)
			printf (" qlen-limit=%ld", qlen_limit.value);
		if (crc4.valid)
			printf (" crc4=%s", crc4.value ? "on" : "off");
		if (higain.valid)
			printf (" higain=%s", higain.value ? "on" : "off");
		if (monitor.valid)
			printf (" monitor=%s", monitor.value ? "on" : "off");
		if (scrambler.valid)
			printf (" scrambler=%s", scrambler.value ? "on" : "off");
	}
	if (timeslots_use.valid && timeslots_use.value)
		printf (" ts=%s", format_timeslots (timeslots_use.value));
	if (timeslots_subchan.valid && timeslots_subchan.value)
		printf (" subchan=%s", format_timeslots (timeslots_subchan.value));
	if (inlevel_sdb.valid)
		printf (" (level=-%.1fdB)", inlevel_sdb.value / 10.0);
	if (ec_delay.valid) {
		int decimals = 0;

		if (ec_delay.value & 4)
			decimals = 1;
		if (ec_delay.value & 2)
			decimals = 2;
		if (ec_delay.value & 1)
			decimals = 3;
		printf (" ec-delay=%.*fms", decimals, ec_delay.value * 0.125);
	}
	printf ("\n");
}

void setup_chan (struct cronyx_item_info_t *node, int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; ++i) {
		if (strcmp ("idle", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "\0\0\0\0\0\0\0");
			need_update_hive = 1;
		} else if (strcmp ("sync", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "sync\0\0\0");
			need_update_hive = 1;
		} else if (strcmp ("cisco", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "cisco\0\0");
			need_update_hive = 1;
		} else if (strcmp ("rbrg", argv[i]) == 0 || strcmp ("bridge", argv[i]) == 0 ||
			   strcmp ("brdg", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "rbrg\0\0\0");
			need_update_hive = 1;
		} else if (strcmp ("raw", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "raw\0\0\0\0");
			need_update_hive = 1;
		} else if (strcmp ("packet", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "packet\0");
			need_update_hive = 1;
		} else if (strcmp ("dahdi", argv[i]) == 0
			   || strcmp ("voip", argv[i]) == 0 || strcmp ("asterisk", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "dahdi\0");
			need_update_hive = 1;
		} else if (strcmp ("zaptel", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "zaptel\0");
			need_update_hive = 1;
#ifdef CRONYX_LYSAP
		} else if (strcmp ("lysap", argv[i]) == 0 || strcmp ("tdmoip", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "lysap\0\0");
			need_update_hive = 1;
#else
		} else if (strcmp ("async", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "async\0\0");
			need_update_hive = 1;
#endif /* CRONYX_LYSAP */
		} else if (strcmp ("fr", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_proto, "fr\0\0\0\0\0");
			need_update_hive = 1;
		} else if (argv[i][0] >= '0' && argv[i][0] <= '9') {
			cronyx_set (node->id, cronyx_baud, strtol (argv[i], 0, 10));
		} else if (strcmp ("extclock", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_baud, 0);
		} else if (is_cmd_param ("adapter", argv[i])) {
			cronyx_set (node->id, cronyx_adapter_mode, get_switch (argv[i],
									       "separate,a\0b-mode,b\0split,c\0mux,d\0",
									       CRONYX_MODE_SEPARATE, CRONYX_MODE_B,
									       CRONYX_MODE_SPLIT, CRONYX_MODE_MUX));
			need_update_hive = 1;
		} else if (is_cmd_param ("debug", argv[i])) {
			cronyx_set (node->id, cronyx_debug, strtol (argv[i] + 6, 0, 10));
		} else if (is_cmd_param ("loop", argv[i])) {
			cronyx_set (node->id, cronyx_loop_mode, get_switch (argv[i],
									    "off,none,normal\0int,internal,digital\0mirror,line-mirror,line\0remote\0",
									    CRONYX_LOOP_NONE,
									    CRONYX_LOOP_INTERNAL,
									    CRONYX_LOOP_LINEMIRROR,
									    CRONYX_LOOP_REMOTE));
		} else if (is_cmd_param ("dpll", argv[i])) {
			cronyx_set (node->id, cronyx_dpll, get_bool_switch (argv[i]));
		} else if (is_cmd_param ("line", argv[i])) {
			cronyx_set (node->id, cronyx_line_code, get_switch (argv[i],
									    "nrz\0nrzi\0hdb3,e1\0ami\0",
									    CRONYX_NRZ, CRONYX_NRZI, CRONYX_HDB3,
									    CRONYX_AMI));
		} else if (is_cmd_param ("invclk", argv[i])) {
			cronyx_set (node->id, cronyx_invclk_mode, get_switch (argv[i],
									      "off,none\0rx,rx-only\0tx,tx-only\0on,both,rx-tx,tx-rx\0",
									      CRONYX_ICLK_NORMAL,
									      CRONYX_ICLK_RX, CRONYX_ICLK_TX,
									      CRONYX_ICLK));
		} else if (is_cmd_param ("higain", argv[i])) {
			cronyx_set (node->id, cronyx_higain, get_bool_switch (argv[i]));
		} else if (is_cmd_param ("mode", argv[i])) {
			cronyx_set (node->id, cronyx_channel_mode, get_switch (argv[i],
									       "async\0hdlc,sync\0raw,phony\0voice\0",
									       CRONYX_MODE_ASYNC, CRONYX_MODE_HDLC,
									       CRONYX_MODE_PHONY, CRONYX_MODE_VOICE));
			need_update_hive = 1;
		} else if (is_cmd_param ("unframed", argv[i])) {
			cronyx_set (node->id, cronyx_unframed, get_bool_switch (argv[i]));
		} else if (is_cmd_param ("scrambler", argv[i])) {
			cronyx_set (node->id, cronyx_scrambler, get_bool_switch (argv[i]));
		} else if (is_cmd_param ("monitor", argv[i])) {
			cronyx_set (node->id, cronyx_monitor, get_bool_switch (argv[i]));
		} else if (is_cmd_param ("cas", argv[i])) {
			cronyx_set (node->id, cronyx_cas_mode, get_switch (argv[i],
									   "off\0"
									   "set,set-strict\0"
									   "pass,pass-strict\0"
									   "cross,cross-strict\0",
									   CRONYX_CASMODE_OFF, CRONYX_CASMODE_SET,
									   CRONYX_CASMODE_PASS, CRONYX_CASMODE_CROSS));
			cronyx_set (node->id,
				cronyx_cas_flags | ((strstr (argv[i], "-strict") != 0) ? 0 : cronyx_flag_canfail),
				get_switch (argv[i],
					"off,set,pass,cross\0"
					"set-strict,pass-strict,cross-strict\0",
					CRONYX_CAS_ITU, CRONYX_CAS_STRICT));
		} else if (is_cmd_param ("crc4", argv[i])) {
			cronyx_set (node->id, cronyx_crc4, get_bool_switch (argv[i]));
		} else if (is_cmd_param ("mtu", argv[i])) {
			cronyx_set (node->id, cronyx_mtu, strtol (argv[i] + 4, 0, 10));
		} else if (is_cmd_param ("qlen", argv[i])) {
			cronyx_set (node->id, cronyx_qlen, strtol (argv[i] + 5, 0, 10));
		} else if (is_cmd_param ("led", argv[i])) {
			int ledmode;
			u32 cadence = 0;

			cronyx_get (node->id, cronyx_led_mode, &ledmode, &cadence);
			ledmode = get_ledmode (argv[i], &cadence);
			cronyx_set (node->id, cronyx_led_mode, ledmode, cadence);
		} else if (is_cmd_param ("clock", argv[i])) {
			cronyx_set (node->id, cronyx_sync_mode, get_switch (argv[i],
									    "int,internal,self\0"
									    "rcv,receive\0"
									    "rcv0\0rcv1\0rcv2\0rcv3\0"
									    "managed\0",
									    CRONYX_E1CLK_INTERNAL,
									    CRONYX_E1CLK_RECEIVE,
									    CRONYX_E1CLK_RECEIVE_CHAN0,
									    CRONYX_E1CLK_RECEIVE_CHAN1,
									    CRONYX_E1CLK_RECEIVE_CHAN2,
									    CRONYX_E1CLK_RECEIVE_CHAN3,
									    CRONYX_E1CLK_MANAGED));
		} else if (is_cmd_param ("ts", argv[i]) || is_cmd_param ("timeslots", argv[i])) {
			cronyx_set (node->id, cronyx_timeslots_use, scan_timeslots (argv[i], 0));
		} else if (is_cmd_param ("subchan", argv[i])) {
			cronyx_set (node->id, cronyx_timeslots_subchan, scan_timeslots (argv[i], 0));
		} else if (is_cmd_param ("dlci", argv[i])) {
			cronyx_set (node->id, cronyx_add_dlci, strtol (argv[i] + 5, 0, 10));
		} else if (is_cmd_param ("iface", argv[i])) {
			cronyx_set (node->id, cronyx_iface_bind, strtol (argv[i] + 6, 0, 10));
		} else if (strcmp ("reset", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_reset, 0);
			need_update_hive = 1;
		} else if (is_cmd_param ("t3-long", argv[i])) {
			cronyx_set (node->id, cronyx_t3_long, get_bool_switch (argv[i]));
		} else if (is_cmd_param ("ec-delay", argv[i])) {
			float delay_ms = 0.0;

			if (strcmp (argv[i] + 9, "auto") == 0)
				cronyx_set (node->id, cronyx_ec_delay, -1);
			else if (sscanf (argv[i] + 9, "%f", &delay_ms) != 1)
				failed ("invalid command line");
			else
				cronyx_set (node->id, cronyx_ec_delay, (long) (delay_ms * 8 + .5));
		} else if (is_cmd_param ("qlen-limit", argv[i])) {
			cronyx_set (node->id, cronyx_qlen_limit, strtol (argv[i] + 11, 0, 10));
		} else if (is_cmd_param ("crc", argv[i])) {
			cronyx_set (node->id, cronyx_crc_mode, get_switch (argv[i],
									   "none,off,no\0"
									   "on,yes,16\0"
									   "32\0",
									   CRONYX_CRC_NONE,
									   CRONYX_CRC_16, CRONYX_CRC_32));
		} else if (is_cmd_param ("sflg", argv[i])) {
			cronyx_set (node->id, cronyx_hdlc_flags, get_switch (argv[i],
									     "off,no\0"
									     "on,yes,share\0",
									     CRONYX_HDLC_2FLAGS, CRONYX_HDLC_SHARE));
		} else if (is_cmd_param ("cas-strict", argv[i])) {
			cronyx_set (node->id, cronyx_cas_flags, get_switch (argv[i],
									     "off,no\0"
									     "on,yes\0",
									     CRONYX_CAS_ITU, CRONYX_CAS_STRICT));
		} else if (strcmp ("up", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_iface_updown, 1);
		} else if (strcmp ("down", argv[i]) == 0) {
			cronyx_set (node->id, cronyx_iface_updown, 0);
#ifdef CRONYX_LYSAP
		} else if (strncasecmp ("lysap.", argv[i], 6) == 0) {
			int arg_done = lysap_specific (node, argc - i, argv + i);

			if (arg_done < 0)
				exit (arg_done);
			i += arg_done;
#endif
		} else {
			fprintf (stderr, "sconfig: unknown keyword '%s'\n", argv[i]);
			failed ("command-line");
		}
	}
}

void process_item (struct cronyx_item_info_t *node)
{
	if (iflag) {
		print_chan (node);
		print_ifconfig (node);
	}
	if (sflag | xflag)
		print_stats_rs (node);
	if (mflag)
		print_modems (node);
	if (eflag)
		print_e1_stats (node);
	if (tflag)
		print_e3_stats (node);
	if (cflag)
		clear_stats (node->id);

	if (0 == (iflag | sflag | xflag | mflag | eflag | tflag | cflag | flag_roadmap | list_mode))
		print_chan (node);
}

int main (int argc, char **argv)
{
	int i, j;

#ifdef CRONYX_LYSAP
	if (lysap_init ())
		failed ("lysap_init");
#endif /* CRONYX_LYSAP */

	for (j = 0, i = 1; i < argc; ) {
		if (strcmp ("--help", argv[i]) == 0) {
			usage ();
		} else if (strcmp ("--list-channels", argv[i]) == 0) {
			list_mode = CRONYX_ITEM_CHANNEL;
		} else if (strcmp ("--list-interfaces", argv[i]) == 0) {
			list_mode = CRONYX_ITEM_INTERFACE;
		} else if (strcmp ("--list-adapters", argv[i]) == 0) {
			list_mode = CRONYX_ITEM_ADAPTER;
		} else {
			i++;
			continue;
		}
		argv[i] = argv[i + ++j];
		argc--;
	}

	for (;;) {
		switch (getopt (argc, argv, "rmseftucviaxq"
#ifdef CRONYX_LYSAP
				"lLk"
#endif /* CRONYX_LYSAP */
			)) {
			case EOF:
				break;
			case 'q':
				++flag_quiet;
				continue;
			case 'r':
				++flag_roadmap;
				continue;
			case 'a':
				++flag_show_all;
				continue;
			case 'm':
				++mflag;
				continue;
			case 's':
				++sflag;
				continue;
			case 'e':
				++eflag;
				continue;
			case 'f':
				++eflag;
				++fflag;
				continue;
			case 't':
				++tflag;
				continue;
			case 'u':
				++tflag;
				++uflag;
				continue;
			case 'c':
				++cflag;
				continue;
			case 'v':
				++vflag;
				continue;
			case 'i':
				++iflag;
				continue;
			case 'x':
				++xflag;
				continue;
#ifdef CRONYX_LYSAP
			case 'L':
				flag_lysap_info = 2;
				continue;
			case 'l':
				flag_lysap_info = 1;
				continue;
			case 'k':
				++flag_lysap_fibers_info;
				continue;
#endif /* CRONYX_LYSAP */
			default:
				usage ();
		}
		break;
	}
	argc -= optind;
	argv += optind;

	open_binder ();
	if (vflag) {
		char buffer[256];

		printf ("Cronyx sconfig version: 6.1\n");
		if (cronyx_ioctl (binder_fd, CRONYX_BUNDLE_VER, &buffer, 0) < 0)
			failed ("binder.get_bundle_version");
		printf ("Cronyx drivers bundle version: %s\n", buffer);
	}

	hivedir_update (load_roadmap ());
	if (flag_roadmap) {
		if (roadmap_items == 0)
			printf ("sconfig: no any node registered on binder\n");
		else
			print_roadmap (0, 0);
	}
	if (list_mode)
		list_items ();

	if (argc <= 0) {
		for (i = 0; i < roadmap_items; i++)
			process_item (binder_roadmap + i);
	} else {
		struct cronyx_item_info_t item_info;

		memset (&item_info, 0, sizeof (item_info));
		strncpy (item_info.name, argv[0], CRONYX_ITEM_MAXNAME);
		if (cronyx_ioctl (binder_fd, CRONYX_ITEM_INFO, &item_info, 0) < 0)
			failed ("binder.binder_lookup_item");

		argc--;
		argv++;

		if (argc > 0)
			setup_chan (&item_info, argc, argv);
		else if (mflag | sflag | eflag | fflag | tflag | uflag | xflag)
			silently_failed_get_request = 0;

		if (!flag_quiet) {
			process_item (&item_info);
#ifdef CRONYX_LYSAP
			if (flag_lysap_info)
				lysap_print_trunk (&item_info, flag_lysap_info - 1);
			if (flag_lysap_fibers_info)
				lysap_print_fibers_info (&item_info);
#endif /* CRONYX_LYSAP */
		}
	}

	if (need_update_hive)
		hivedir_update (load_roadmap ());

	close (binder_fd);
#ifdef CRONYX_LYSAP
	lysap_close ();
#endif
	return global_error_code;
}
