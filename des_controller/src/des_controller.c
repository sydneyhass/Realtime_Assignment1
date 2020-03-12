#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <sys/netmgr.h>
#include <unistd.h>
#include <errno.h>

#include "../../des_inputs/src/des-mva.h"

typedef void *(*StateFunc)();

//state function pointers
void *start_state(Person *p);
void *scan_state(Person *p, Display *display);
void *unlock_state(Person *p, Display *display);
void *open_state(Person *p, Display *display);
void *lock_state(Person *p, Display *display);
void *close_state(Person *p, Display *display);

void *start_state(Person *p) {

	// Check for scan state, no message made as display creates it
	if (p->state == SCAN_STATE)
		return scan_state;

	// Invalid state, make person state start for the purpose of display
	// return no message, console would just display menu again
	p->state = START_STATE;
	return start_state;
}

void *scan_state(Person *p, Display *display) {

	if (p->state == UNLOCK_STATE) {
		display->outMessage = GLU_MSG;
		return unlock_state;
	}

	// if close from scan, close starting door
	if (p->state == CLOSE_STATE) {
		display->outMessage = LC_MSG;
		return close_state;
	}

	p->state = START_STATE;
	return scan_state;
}

void *unlock_state(Person *p, Display *display) {

	// If person state is open, left open and progress to open state
	if (p->state == OPEN_STATE && p->direction == INBOUND) {
		display->outMessage = LO_MSG;
		return open_state;
	}
	if (p->state == OPEN_STATE) {
		display->outMessage = RO_MSG;
		return open_state;
	}

	// No output message and remain in unlock state
	display->outMessage = ERR_MSG;
	return unlock_state;

}

void *open_state(Person *p, Display *display) {
	//Person will be scan state when being weighed, let display make message
	if (p->state == SCAN_STATE)
		return scan_state;
	//Close left if inbound, right if outbound
	if (p->state == CLOSE_STATE && p->direction == INBOUND) {
		display->outMessage = LC_MSG;
		return close_state;
	}
	if (p->state == CLOSE_STATE) {
		display->outMessage = RC_MSG;
		return close_state;
	}
	display->outMessage = ERR_MSG;
	return open_state;
}

void *lock_state(Person *p, Display *display) {
	// Unlock r if inbound
	if (p->state == UNLOCK_STATE && p->direction == INBOUND) {
		display->outMessage = GRU_MSG;
		return unlock_state;
	}
	if (p->state == UNLOCK_STATE) {
		display->outMessage = WAIT_MSG;
		return start_state;
	}
	display->outMessage = ERR_MSG;
	return lock_state;
}

void *close_state(Person *p, Display *display) {
	if (p->state == LOCK_STATE && p->direction == INBOUND) {
		display->outMessage = GLL_MSG;
		return lock_state;
	}
	if (p->state == LOCK_STATE) {
		display->outMessage = GRL_MSG;
		return lock_state;
	}
	display->outMessage = ERR_MSG;
	return close_state;
}

int main(int argc, char* argv[]) {
	pid_t displaypid;
	int coid;
	int chid;
	int rcvid;
	Person person;
	Display display;

	StateFunc statefunc = start_state;

	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments\n");
		exit(EXIT_FAILURE);
	}

	displaypid = atoi(argv[1]);

	if ((chid = ChannelCreate(0)) == -1) {
		fprintf(stderr, "Controller error creating channel");
		exit(EXIT_FAILURE);
	}

	//TODO: Attach to display - need nid, chid, coid
	if ((coid = ConnectAttach(ND_LOCAL_NODE, displaypid, 1, _NTO_SIDE_CHANNEL, 0)) == -1) {
		fprintf(stderr, "Controller ConnectAttach error\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	while (1) {
		if ((rcvid = MsgReceive(chid, &person, sizeof(person), NULL)) < 0) {
			perror("Controller's MsgReceive error.");
			return EXIT_FAILURE;
		}

		// Call state and assign next state
		statefunc = (StateFunc) (*statefunc)(&person, &display);

		// Add person to display struct
		display.person = person;
		if (MsgSend(displaypid, &display, sizeof(display), NULL, 0) == -1) {
			fprintf(stderr, "Controller's MsgSend had an error\n");
			exit(EXIT_FAILURE);
		}

		if (MsgReply(rcvid, EOK, NULL, 0) == -1) {
			perror("Controller's MsgReply error.");
			exit(EXIT_FAILURE);
		}

		// Can just break here as above msgSend will send person.state = exit to Display which will trigger break condition there
		if (person.state == EXIT_STATE)
			break;

	}

	ConnectDetach(coid);
	ChannelDestroy(chid);

	return EXIT_SUCCESS;
}
