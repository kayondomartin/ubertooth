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
#include <ctype.h>
#include <err.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "ubertooth.h"
#include "ubertooth_callback.h"
#include "procData.h"
#include "bchEcc.h"
#include "aesAlg.h"

#define BLOCK_SIZE 16
#define FREAD_COUNT 4096


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

	slen = strlen(s);
	if (slen % 2 == 1)
		printf("Discard last word\n");
	printf("data: ");
	for (i=0; i < slen-1; i+=2) {
		if (!isxdigit(s[i])) {
			printf("Error: input data contains invalid character(s)\n");
			return 0;
		}
		unsigned byte;
		sscanf(&s[i], "%02x", &byte);
		o[i/2] = byte;
		printf("%02x ", o[i/2]);
	}
	
	printf("\n");
	return 1;
}

int syncStart(char *APMAC, int do_adv_index, ubertooth_t *ut, int ubertooth_device, int *time, int *rssi) {
	int status, i, ofsRssi, ofsTime, dataLen;
	u16 channel;
	uint8_t mac_address[6] = { 0, };

	if (do_adv_index == 37)
		channel = 2402;
	else if (do_adv_index == 38)
		channel = 2426;
	else
		channel = 2480;
	cmd_set_channel(ut->devh, channel);

	status = convert_mac_address(APMAC, mac_address);
	printf("AP MAC address: %s\n", APMAC);
	cmd_btle_slave(ut->devh, mac_address, UBERTOOTH_BTLE_SYNC, 6);
	struct timespec tspec;
	clock_gettime(CLOCK_MONOTONIC, &tspec);
	uint64_t sync_start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
	uint64_t start = 0;

	usleep(1000);
	usb_pkt_rx rx;
	ofsRssi = 0;
	ofsTime = 0;
	while(1) {
		int r = cmd_poll(ut->devh, &rx);
		if (r < 0) {
			printf("USB Error\n");
			break;
		}
		if (r == sizeof(usb_pkt_rx)) {
			fifo_push(ut->fifo, &rx);
			if (start == 0) {
				clock_gettime(CLOCK_MONOTONIC, &tspec);
				start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
			}
			rssi_sampling(ut, rssi, ofsRssi);
			ofsRssi += 50;
			int time_count = 4;
			while (--time_count) {
				int m = cmd_poll(ut->devh, &rx);
				if (m < 0) {
					printf("USB Error\n");
					break;
				}
				if (m == sizeof(usb_time_rx)) {
					fifo_push(ut->fifo, &rx);
					time_sampling(ut, time, ofsTime);
					ofsTime += 16;
				}
			}
			int m = cmd_poll(ut->devh, &rx);
			if (m < 0) {
				printf("USB Error\n");
				break;
			}
			if (m == sizeof(usb_time_rx)) {
				fifo_push(ut->fifo, &rx);
				time_sampling_last(ut, time, ofsTime);
				ofsTime += 2;
			}
		}
		if (start != 0) {
			clock_gettime(CLOCK_MONOTONIC, &tspec);
			uint64_t now = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
			if (now - start > 127)
				break;
		}
		usleep(500);
	}
	dataLen = ofsRssi;
	ubertooth_stop(ut);


	free(ut);
	ut = ubertooth_init();
	status = ubertooth_connect(ut, ubertooth_device);
	if (status < 0) {
		return 1;
	}

	status = ubertooth_check_api(ut);
	if (status < 0)
		return 1;

	register_cleanup_handler(ut, 1);

	return dataLen;
}
	
int dataTx(char *APMAC, int *bch, int rLen, char *encPwd, int txDur, ubertooth_t *ut, int ubertooth_device) {
	int status, i, pwdLen, dlen;
	uint32_t parity = 0x00000000;
	uint8_t errCap = 0x00, *parityByte;
	int *eccInt;
	uint8_t mac_address[6] = {0, };

	status = convert_mac_address(APMAC, mac_address);

	parityByte = (uint8_t *)&parity;

	if (rLen == 0)
		errCap = 0x00;
	else if (rLen == 14)
		errCap = 0x02;
	else
		errCap = 0x04;

	eccInt = (int*)malloc(sizeof(int)*rLen);
	for(i=0; i<rLen; i++) {
		eccInt[i] = bch[127 - rLen + i];
	}
	for(i=0; i<rLen; i++) {
		parity <<= 1;
		parity |= eccInt[i];
	}

	pwdLen = strlen(encPwd);
	dlen = 5 + pwdLen;
	uint8_t *tot_data = (uint8_t*) malloc(sizeof(uint8_t) * (dlen + 6));
	for(i=0; i<6; i++) tot_data[i] = mac_address[i];
	tot_data[6] = errCap;
	for(i=7; i< (7 + 4); i++) tot_data[i] = parityByte[3-i+7];
	tot_data[11] = 0xcc; 
	for(i=12; i<pwdLen+12; i++) tot_data[i] = (uint8_t)encPwd[i-12];

	printf("Broadcast data: ");
	for(i=0; i<pwdLen+11; i++) printf("%02x ", tot_data[i]);
	printf("\n");
	cmd_btle_slave(ut->devh, tot_data, UBERTOOTH_BTLE_SLAVE, dlen+6);
	usleep(txDur * 1000);
	ubertooth_stop(ut);
	
	free(ut);
	ut = ubertooth_init();
	status = ubertooth_connect(ut, ubertooth_device);
	if (status < 0) 
		return 1;
	
	status = ubertooth_check_api(ut);
	if (status < 0)
		return 1;
	register_cleanup_handler(ut, 1);
	free(eccInt);
	free(tot_data);

	return status;
}

int scanOK(int rxDur, ubertooth_t *ut, int ubertooth_device) {
	int status, ok = 0;
	u16 channel = 2402;
	usb_pkt_rx rx;

	cmd_set_jam_mode(ut->devh, JAM_NONE);
	cmd_set_modulation(ut->devh, MOD_BT_LOW_ENERGY);
	cmd_set_channel(ut->devh, channel);
	cmd_btle_sniffing(ut->devh, 2);

	struct timespec tspec;
	uint64_t start = 0, now = 0;
	clock_gettime(CLOCK_MONOTONIC, &tspec);
	start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
	now = start;
	while(now - start < rxDur) {
		int r = cmd_poll(ut->devh, &rx);
		if (r < 0) {
			printf("USB Error\n");
			break;
		}
		if (r == sizeof(usb_pkt_rx)) {
			fifo_push(ut->fifo, &rx);
			ok = find_OK(ut);
			if (ok == 1)
				break;
		}
		usleep(500);
		clock_gettime(CLOCK_MONOTONIC, &tspec);
		now = (tspec.tv_sec) * 1000 + (tspec.tv_nsec)/1000000;
	}
	ubertooth_stop(ut);

	free(ut);
	ut = ubertooth_init();
	status = ubertooth_connect(ut, ubertooth_device);
	if (status < 0) {
		return status;
	}
	status = ubertooth_check_api(ut);
	if (status < 0)
		return status;
	register_cleanup_handler(ut, 1);
	
	return ok;
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
	printf("\t-S Rssi sampling synchronization mode\n");
	printf("\t-T BLE scanning duration\n");
	printf("\t-H BLE IoT connection HOST mode\n");
	printf("\t-G BLE IoT connection GUEST mode\n");

	printf("\nIf an input file is not specified, an Ubertooth device is used for live capture.\n");
	printf("In get/set mode no capture occurs.\n");
}

int main(int argc, char *argv[])
{
	//////
	int opt;
	int do_follow, do_promisc, do_cfo, do_rssi;
	int do_get_aa, do_set_aa;
	int do_crc;
	int do_adv_index;
	int do_slave_mode, do_data_mode, do_sync_mode;
	int do_host, do_guest;
	// JWHUR set advertising data
	int duration; // ms
	uint8_t *data;
	int dlen;
	int do_target;
	enum jam_modes jam_mode = JAM_NONE;
	int ubertooth_device = -1;
	ubertooth_t* ut = (ubertooth_t*)malloc(sizeof(ubertooth_t));
	ut = ubertooth_init();

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
	duration = 3000;

	while ((opt=getopt(argc,argv,"a::r:hfoRpU:v::A:s:d:S:T:l:t:x:H:G:c:o:q:jJiI")) != EOF) {
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
			dlen = strlen(optarg);
			data = (uint8_t*) malloc(sizeof(uint8_t) * dlen);
			r = convert_data(optarg, data);
			if (r == 0)
				return;
			do_data_mode = 1;
			break;
		//JWHUR rssi sampling synchronization mode
		case 'S':
			do_sync_mode = 1;
			break;
		case 'T':
			duration = (int) atoi(optarg);
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
//JWHUR BLE IoT connection
		case 'H':
			do_host = 1;
			break;
		case 'G':
			do_guest = 1;
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

		clock_gettime(CLOCK_MONOTONIC, &tspec);
		start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;

		while (1) {
			int r = cmd_poll(ut->devh, &rx);
			if (r < 0) {
				printf("USB error\n");
				break;
			}
			if (r == sizeof(usb_pkt_rx)) {
				fifo_push(ut->fifo, &rx);
				if(!do_rssi) sync = cb_btle(ut, &cb_opts);
				if(do_rssi == 0) {
					if (sync == 1) {
						clock_gettime(CLOCK_MONOTONIC, &tspec);
						sync_start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
						do_rssi = 1;
						start = 0;
					}
				} else {
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
			if (start != 0) {
				clock_gettime(CLOCK_MONOTONIC, &tspec);
				uint64_t now = (tspec.tv_sec) * 1000 + (tspec.tv_nsec)/1000000;
				if (do_rssi*(now - start) > 127 || now - start > duration)
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
			usleep(duration * 1000);
			ubertooth_stop(ut);
		} else if (do_sync_mode) {	
			printf("\n%u:%u:%u:%u:%u:%u\n", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
			cmd_btle_slave(ut->devh, mac_address, UBERTOOTH_BTLE_SYNC, 6);
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
					if (now - start > 127)
						break;
				}		
				usleep(500);
			}
			ubertooth_stop(ut);
			
		} else 
			cmd_btle_slave(ut->devh, tot_data, UBERTOOTH_BTLE_SLAVE, dlen+6);
		free(tot_data);
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

	if (do_host) {
		int status, rLen, nCor, txDur, rxDur, i, lenData;
		int *Barcode, *bch;
		char encPwd[FREAD_COUNT + BLOCK_SIZE], decPwd[FREAD_COUNT + BLOCK_SIZE];
		char APSSID[100] = "", APPWD[100] = "", APMAC[17] = "";
		struct timespec tspec;
		uint64_t start, now;

		srand(time(NULL));
		Barcode = malloc(sizeof(int)*127);
		bch = malloc(sizeof(int)*127);

		status = getAPInfo(APMAC, APSSID, APPWD);
		printf("APSSID: %s, APMAC: %s, APPWD: %s\n", APSSID, APMAC, APPWD);
		clock_gettime(CLOCK_MONOTONIC, &tspec);
		start = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;
		now = start;
		while (now - start < 3000) {
			int time[2540] = {0, }, rssi[2540] = {0, };
			lenData = syncStart(APMAC, do_adv_index, ut, ubertooth_device, time, rssi);
			Barcode = procData(time, rssi, lenData);
			printf("\nBarcode: ");
			for(i=0; i<127; i++)
				printf("%d ", Barcode[i]);
			printf("\n");

			nCor = 0;
			rLen = makeBCH(Barcode, bch);
			printf("\nBCH enc.: ");
			for(i=0; i<127; i++)
				printf("%d ", bch[i]);
			printf("\n");

			status = aesEncrypt(bch, APPWD, encPwd);
			txDur = 50 + (rand()%10 - 10);
			rxDur = 100 - txDur;
			status = dataTx(APMAC, bch, rLen, encPwd, txDur, ut, ubertooth_device);
			status = scanOK(rxDur, ut, ubertooth_device);
			if (status == 1) {
				printf("IoT WiFi connection success!\n");
				break;	// IoT WiFi connection finish
			}
			
			clock_gettime(CLOCK_MONOTONIC, &tspec);
			now = (tspec.tv_sec)*1000 + (tspec.tv_nsec)/1000000;

		}
	}

	if (!(do_follow || do_promisc || do_get_aa || do_set_aa ||
				do_crc >= 0 || do_slave_mode || do_target || do_host || do_guest))
		usage();

	return 0;
}
