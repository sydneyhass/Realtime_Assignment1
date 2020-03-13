#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "../../des_controller/src/des-mva.h"

int main(void) {
	int chid;
	int rcvid;
	Display display;

	if ((chid = ChannelCreate(0)) == -1) {
		perror("Display ChannelCreate error.\n");
		exit(EXIT_FAILURE);
	}

	printf("The display is running as PID: %d\n", getpid());

	while (1) {

		if ((rcvid = MsgReceive(chid, &display, sizeof(Display), NULL)) < 0) {
			perror("Display MsgRecieve error.\n");
			exit(EXIT_FAILURE);
		}
		if (display.person.state == SCAN_STATE && !display.person.weight && display.person.personID) {
			printf("Person scanned ID, ID = %d\n", display.person.personID);
		}

		else if (display.person.state == SCAN_STATE) {
			printf("Person weighed, weight = %d\n", display.person.weight);
		} else if(display.person.state == EXIT_STATE) {
			break;
		}
		else {
			printf("%s\n", outMessage[display.outMessage]);
		}
		if(display.person.state == START_STATE && display.person.direction == OUTBOUND) {
			printf("%s\n", outMessage[WAIT_MSG]);
		}
		if (MsgReply(rcvid, EOK, NULL, 0) == -1) {
			perror("Display MsgReply error.\n");
			exit(EXIT_FAILURE);
		}
	}

	if (ChannelDestroy(chid) == -1) {
		perror("Display ChannelDestroy error.\n");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
