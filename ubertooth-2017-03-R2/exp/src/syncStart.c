#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "syncStart.h"

int syncStart(char *macAP) {
	int status;
	char *pre_com, *post_com, *command;

	command = malloc(sizeof(char)*100);
	pre_com = "ubertooth-btle -s ";
	post_com = " -S -U 0 > log";

	strcat(command, pre_com);
	strcat(command, macAP);
	strcat(command, post_com);
	
	status = system(command); // ubertooth-btle -s macAP -s -U 0 >& log
	status = system("catTime=$(sed -n -e 's/^.*measurement : //p' log); echo $catTime > time; tr -s ' ' '\n'< time > time.dat; rm time");
	status = system("catRssi=$(sed -n -e 's/^.*samples : //p' log); echo $catRssi > rssi; tr -s ' ' '\n'< rssi > rssi.dat; rm rssi");

	return status;
}
