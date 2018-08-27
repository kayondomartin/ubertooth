/*
 * Copyright 2012-2016 Michael Ryan
 *
 * This file is part of Project Ubertooth.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "ubertooth.h"
#include "ubertooth_callback.h"
#include <ctype.h>
#include <err.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/* To automatically kill this process after 100ms */
#include <sys/types.h>
#include <sys/stat.h>
#include <error.h>
#include <signal.h>
#include <syslog.h>
#include <stdio.h>

int convert_mac_address(char *s, uint8_t *o) {
	int i;

	// validate length
	if (strlen(s) != 6 * 2 + 5) {
		printf("Error: MAC address is wrong length\n");
		return 0;
	}

	// validate hex chars and : separators
	for (i = 0; i < 6*3; i += 3) {
		if (!isxdigit(s[i]) ||
			!isxdigit(s[i+1])) {
			printf("Error: MAC address contains invalid character(s)\n");
			return 0;
		}
		if (i < 5*3 && s[i+2] != ':') {
			printf("Error: MAC address contains invalid character(s)\n");
			return 0;
		}
	}

	// sanity: checked; convert
	for (i = 0; i < 6; ++i) {
		unsigned byte;
		sscanf(&s[i*3], "%02x",&byte);
		o[i] = byte;
	}

	return 1;
}

//JWHUR set advertising data
int convert_data(char *s, uint8_t *o) {
	int i, slen;

	printf("data: ");
	slen = strlen(s);
	for (i=0; i < slen; i++) {
		uint8_t byte;
		sscanf(&s[i], "%c", &byte);
		o[i] = byte;
		printf("%u ", o[i]);
	}
	printf("\n");
	return 1;
}

static void usage(void)
{
	printf("ubertooth-btle - passive Bluetooth Low Energy monitoring\n");
	printf("Usage:\n");
	printf("\t-h this help\n");
	printf("\n");
	printf("    Major modes:\n");
	printf("\t-f follow connections\n");
	printf("\t-p promiscuous: sniff active connections\n");
	printf("\t-a[address] get/set access address (example: -a8e89bed6)\n");
	printf("\t-s<address> faux slave mode, using MAC addr (example: -s22:44:66:88:aa:cc)\n");
	printf("\t-t<address> set connection following target (example: -t22:44:66:88:aa:cc)\n");
	printf("\n");
	printf("    Interference (use with -f or -p):\n");
	printf("\t-i interfere with one connection and return to idle\n");
	printf("\t-I interfere continuously\n");
	printf("\n");
	printf("    Data source:\n");
	printf("\t-U<0-7> set ubertooth device to use\n");
	printf("\n");
	printf("    Misc:\n");
	printf("\t-r<filename> capture packets to PCAPNG file\n");
	printf("\t-q<filename> capture packets to PCAP file (DLT_BLUETOOTH_LE_LL_WITH_PHDR)\n");
	printf("\t-c<filename> capture packets to PCAP file (DLT_PPI)\n");
	printf("\t-A<index> advertising channel index (default 37)\n");
	printf("\t-v[01] verify CRC mode, get status or enable/disable\n");
	printf("\t-x<n> allow n access address offenses (default 32)\n");
	printf("\t-d set ubertooth advertising data\n");
	printf("\t-o center frequency offset tracking\n");
	printf("\t-R RSSI tracking\n");
	printf("\t-S Rssi sampling synchronization mode");

	printf("\nIf an input file is not specified, an Ubertooth device is used for live capture.\n");
	printf("In get/set mode no capture occurs.\n");
}

int main(int argc, char *argv[])
{
	/* To kill this process */
	FILE *fp;
	char line[1035], pid_string[1035];
	int pid = 0, i;
	//////
	int opt;
	int do_follow, do_promisc, do_cfo, do_rssi;
	int do_get_aa, do_set_aa;
	int do_crc;
	int do_adv_index;
	int do_slave_mode, do_data_mode, do_sync_mode;
	// JWHUR set advertising data
	uint8_t *data;
	int dlen;
	int do_target;
	enum jam_modes jam_mode = JAM_NONE;
	int ubertooth_device = -1;
	ubertooth_t* ut = ubertooth_init();

	btle_options cb_opts = { .allowed_access_address_errors = 32 };

	int r;
	u32 access_address;
	uint8_t mac_address[6] = { 0, };

	do_follow = do_promisc = 0;
	do_cfo = 0;
	do_rssi = 0;
	do_get_aa = do_set_aa = 0;
	do_crc = -1; // 0 and 1 mean set, 2 means get
	do_adv_index = 37;
	do_slave_mode = do_target = do_data_mode = do_sync_mode = 0;
	dlen = 0;

	while ((opt=getopt(argc,argv,"a::r:hfoRpU:v::A:s:d:S:l:t:x:c:o:q:jJiI")) != EOF) {
		switch(opt) {
		case 'a':
			if (optarg == NULL) {
				do_get_aa = 1;
			} else {
				do_set_aa = 1;
				sscanf(optarg, "%08x", &access_address);
			}
			break;
		case 'f':
			do_follow = 1;
			break;
		case 'o':
			do_cfo = 1;
		case 'R':
			do_rssi = 1;
		case 'p':
			do_promisc = 1;
			break;
		case 'U':
			ubertooth_device = atoi(optarg);
			break;
		case 'r':
			if (!ut->h_pcapng_le) {
				if (lell_pcapng_create_file(optarg, "Ubertooth", &ut->h_pcapng_le)) {
					err(1, "lell_pcapng_create_file: ");
				}
			}
			else {
				printf("Ignoring extra capture file: %s\n", optarg);
			}
			break;
		case 'q':
			if (!ut->h_pcap_le) {
				if (lell_pcap_create_file(optarg, &ut->h_pcap_le)) {
					err(1, "lell_pcap_create_file: ");
				}
			}
			else {
				printf("Ignoring extra capture file: %s\n", optarg);
			}
			break;
		case 'c':
			if (!ut->h_pcap_le) {
				if (lell_pcap_ppi_create_file(optarg, 0, &ut->h_pcap_le)) {
					err(1, "lell_pcap_ppi_create_file: ");
				}
			}
			else {
				printf("Ignoring extra capture file: %s\n", optarg);
			}
			break;
		case 'v':
			if (optarg)
				do_crc = atoi(optarg) ? 1 : 0;
			else
				do_crc = 2; // get
			break;
		case 'A':
			do_adv_index = atoi(optarg);
			if (do_adv_index < 37 || do_adv_index > 39) {
				printf("Error: advertising index must be 37, 38, or 39\n");
				usage();
				return 1;
			}
			break;
		case 's':
			do_slave_mode = 1;
			r = convert_mac_address(optarg, mac_address);
			if (!r) {
				usage();
				return 1;
			}
			break;
		//JWHUR set advertising data
		case 'd':
			do_data_mode = 1;
			dlen = strlen(optarg);
			data = (uint8_t*) malloc(sizeof(uint8_t) * dlen);
			r = convert_data(optarg, data);
			break;
		//JWHUR rssi sampling synchronization mode
		case 'S':
			do_sync_mode = 1;
			break;
		case 't':
			do_target = 1;
			r = convert_mac_address(optarg, mac_address);
			if (!r) {
				usage();
				return 1;
			}
			break;
		case 'x':
			cb_opts.allowed_access_address_errors = (unsigned) atoi(optarg);
			if (cb_opts.allowed_access_address_errors > 32) {
				printf("Error: can tolerate 0-32 access address bit errors\n");
				usage();
				return 1;
			}
			break;
		case 'i':
		case 'j':
			jam_mode = JAM_ONCE;
			break;
		case 'I':
		case 'J':
			jam_mode = JAM_CONTINUOUS;
			break;
		case 'h':
		default:
			usage();
			return 1;
		}
	}


	r = ubertooth_connect(ut, ubertooth_device);
	if (r < 0) {
		usage();
		return 1;
	}

	r = ubertooth_check_api(ut);
	if (r < 0)
		return 1;

	/* Clean up on exit. */
	register_cleanup_handler(ut, 1);

	if (do_follow && do_promisc) {
		printf("Error: must choose either -f or -p, one or the other pal\n");
		return 1;
	}

	// JWHUR add do_cfo/ do_rssi
	if (do_follow || do_promisc || do_cfo || do_rssi) {
		usb_pkt_rx rx;

		r = cmd_set_jam_mode(ut->devh, jam_mode);
		if (jam_mode != JAM_NONE && r != 0) {
			printf("Jamming not supported\n");
			return 1;
		}
		cmd_set_modulation(ut->devh, MOD_BT_LOW_ENERGY);

		if (do_follow || do_cfo || do_rssi) {
			u16 channel;
			if (do_adv_index == 37)
				channel = 2402;
			else if (do_adv_index == 38)
				channel = 2426;
			else
				channel = 2480;
			cmd_set_channel(ut->devh, channel);
			if (do_follow) cmd_btle_sniffing(ut->devh, 2);
			if (do_cfo) cmd_btle_tracking(ut->devh, 2, 0); // cfo tracking == 0 flag
			if (do_rssi) cmd_btle_tracking(ut->devh, 2, 1); // rssi tracking == 1 flag
		} else {
			cmd_btle_promisc(ut->devh);
		}
	
		int sync = 0; //JWHUR test for synchronization
		struct timespec tspec;
		uint64_t sync_start, start = 0;
		while (1) {
			int r = cmd_poll(ut->devh, &rx);
			if (r < 0) {
				printf("USB error\n");
				break;
			}
			if (r == sizeof(usb_pkt_rx)) {
				fifo_push(ut->fifo, &rx);
				if(!do_rssi) sync = cb_btle(ut, &cb_opts);
				if(sync == 1 && do_rssi == 0) {
					clock_gettime(CLOCK_MONOTONIC, &tspec);
					sync_start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
					do_rssi = 1;
				} else if(do_rssi == 1) {
					if (start == 0) {
						clock_gettime(CLOCK_MONOTONIC, &tspec);
						start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
					}
					cb_btle_tracking(ut, &cb_opts);
					printf("time measurement : ");
					int time_count = 4;
					while (--time_count) {
						int m = cmd_poll(ut->devh, &rx);
						if (m<0) {
							printf("USB error \n");
							break;
						}
						if (m == sizeof(usb_time_rx)) {
							fifo_push(ut->fifo, &rx);
							cb_btle_time(ut, &cb_opts);
						}
					}
					int m = cmd_poll(ut->devh, &rx);
					if (m<0) {
						printf("USB error \n");
						break;
					}
					if (m == sizeof(usb_time_rx)) {					
						fifo_push(ut->fifo, &rx);
						cb_btle_time_last(ut, &cb_opts);
					}
				}
				if (do_cfo) {
					int m = cmd_poll(ut->devh, &rx);
					if (m < 0) {
						printf("USB error \n");
						break;
					}
					if (m == sizeof(usb_pkt_rx)) {
						fifo_push(ut->fifo, &rx);
						cb_btle_tracking(ut, &cb_opts);
					}
				}
			}
			usleep(500);
			if (do_rssi == 1 && start != 0) {
				clock_gettime(CLOCK_MONOTONIC, &tspec);
				uint64_t now = (tspec.tv_sec) * 1000 + (tspec.tv_nsec)/1000000;
				if (now - start > 100)
					break;
			}
		}
		ubertooth_stop(ut);
	}

	if (do_get_aa) {
		access_address = cmd_get_access_address(ut->devh);
		printf("Access address: %08x\n", access_address);
		return 0;
	}

	if (do_set_aa) {
		cmd_set_access_address(ut->devh, access_address);
		printf("access address set to: %08x\n", access_address);
	}

	if (do_crc >= 0) {
		int r;
		if (do_crc == 2) {
			r = cmd_get_crc_verify(ut->devh);
		} else {
			cmd_set_crc_verify(ut->devh, do_crc);
			r = do_crc;
		}
		printf("CRC: %sverify\n", r ? "" : "DO NOT ");
	}

	if (do_slave_mode) {
		u16 channel;
		if (do_adv_index == 37)
			channel = 2402;
		else if (do_adv_index == 38)
			channel = 2426;
		else
			channel = 2480;
		cmd_set_channel(ut->devh, channel);

		//JWHUR add advertising data
		uint8_t *tot_data = (uint8_t*) malloc(sizeof(uint8_t) * (dlen + 6));
		int i;
		for(i=0; i<6; i++) tot_data[i] = mac_address[i];
		if (do_data_mode) {
			for(i=6; i< (dlen + 6); i++) tot_data[i] = data[i-6];
			cmd_btle_slave(ut->devh, tot_data, UBERTOOTH_BTLE_SLAVE, dlen+6);
		} else if (do_sync_mode) {
			cmd_btle_slave(ut->devh, NULL, UBERTOOTH_BTLE_SYNC, 0);
			struct timespec tspec;
			clock_gettime(CLOCK_MONOTONIC, &tspec);
			uint64_t sync_start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
			
			do_rssi = 1;
			uint64_t start = 0;
	
			usleep(1000);
			usb_pkt_rx rx;
			while (1) {
				int r = cmd_poll(ut->devh, &rx);
				if (r < 0) {
					printf("USB error\n");
					break;
				}
				if (r == sizeof(usb_pkt_rx)) {
					fifo_push(ut->fifo, &rx);
					if(do_rssi) {
						if (start == 0) {
							clock_gettime(CLOCK_MONOTONIC, &tspec);
							start = (tspec.tv_sec) * 1000 + (tspec.tv_nsec)/1000000;
						}
						cb_btle_tracking(ut, &cb_opts);
						printf("time measurement : ");
						int time_count = 4;
						while (--time_count) {
						int m = cmd_poll(ut->devh, &rx);
							if (m<0) {
								printf("USB error \n");
								break;
							}
							if (m == sizeof(usb_time_rx)) {
								fifo_push(ut->fifo, &rx);
								cb_btle_time(ut, &cb_opts);
							}
						}
						int m = cmd_poll(ut->devh, &rx);
						if (m<0) {
							printf("USB error \n");
							break;
						}
						if (m == sizeof(usb_time_rx)) {					
							fifo_push(ut->fifo, &rx);
							cb_btle_time_last(ut, &cb_opts);
						}
					}
				}
				if (do_rssi == 1 && start != 0) {
					clock_gettime(CLOCK_MONOTONIC, &tspec);
					uint64_t now = (tspec.tv_sec) * 1000 + (tspec.tv_nsec)/1000000;
					if (now - start > 100)
						break;
				}		
				usleep(500);
			}
			ubertooth_stop(ut);
			
		} else 
			cmd_btle_slave(ut->devh, tot_data, UBERTOOTH_BTLE_SLAVE, dlen+6); 
	}

	if (do_target) {
		r = cmd_btle_set_target(ut->devh, mac_address);
		if (r == 0) {
			int i;
			printf("target set to: ");
			for (i = 0; i < 5; ++i)
				printf("%02x:", mac_address[i]);
			printf("%02x\n", mac_address[5]);
		}
	}

	if (!(do_follow || do_promisc || do_get_aa || do_set_aa ||
				do_crc >= 0 || do_slave_mode || do_target))
		usage();

	return 0;
}
