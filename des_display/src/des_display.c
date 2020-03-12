#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>

#include "../../des_inputs/src/des-mva.h"

int main(void) {
	int chid;
	Display display;
	char rMsg[256];
	int rcvid;

	if ((chid = ChannelCreate(0)) == -1) {
		perror("ChannelCreate error.");
		exit(EXIT_FAILURE);
	}

	while (1) {
		if ((rcvid = MsgReceive(chid, &display, sizeof(display), NULL)) < 0) {
			perror("MesRecieve error.");
			exit(EXIT_FAILURE);
		}

		if (strcmp(rMsg, inMessage[LS_INPUT]) == 0
				|| strcmp(rMsg, inMessage[RS_INPUT]) == 0) {
			printf("Person scanned ID, ID = %d\n", display.person.personID);
		}

		else if (strcmp(rMsg, inMessage[WS_INPUT]) == 0) {
			printf("Person weighed, weight = %d\n", display.person.weight);
		}

		else {
			printf("%s\n", outMessage[0]);
		}
	}

	if (ChannelDestroy(chid) == -1) {
		perror("ChannelDestroy error.");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
