#ifndef BACKUP_RESTORE_SREVICE_H
#define BACKUP_RESTORE_SERVICE_H

#define ARGSLEN 255

typedef struct backup_restore_cmd{
	int action;
	char args[ARGSLEN];
}backup_restore_cmd;

#endif
