#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // 1. Setup syslog logging with LOG_USER facility
    openlog("writer-app", LOG_PID, LOG_USER);

    // 2. Check arguments: Expects [writefile] and [writestr]
    if (argc != 3) {
        syslog(LOG_ERR, "Invalid number of arguments: Expected 2, got %d", argc - 1);
        fprintf(stderr, "Usage: %s <file> <string>\n", argv[0]);
        closelog();
        return 1;
    }

    char *writefile = argv[1];
    char *writestr = argv[2];

    // 3. Log the action at LOG_DEBUG level
    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

    // 4. Perform File I/O
    FILE *fp = fopen(writefile, "w");
    if (fp == NULL) {
        // Log unexpected error with LOG_ERR
        syslog(LOG_ERR, "Failed to open file %s: %s", writefile, strerror(errno));
        perror("Error opening file");
        closelog();
        return 1;
    }

    if (fputs(writestr, fp) == EOF) {
        syslog(LOG_ERR, "Failed to write to file %s: %s", writefile, strerror(errno));
        fclose(fp);
        closelog();
        return 1;
    }

    // 5. Cleanup
    fclose(fp);
    closelog();

    return 0;
}
