#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <regex.h>

char* getPidCommand(int pid) {
    // Get process status from /proc/<pid>/stat
    // Get command line from /proc/<pid>/cmdline
    char cmdline_path[256];
    snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", pid);

    FILE *cmdline_file = fopen(cmdline_path, "r");
    if (!cmdline_file) {
        perror("Failed to open process command line file");
        return NULL;
    }

    char *cmdline = malloc(4096);  // Dynamically allocated memory
    size_t length = fread(cmdline, 1, 4096, cmdline_file);

    if (length > 0) {
        // Replace null bytes with spaces to separate arguments
        for (size_t i = 0; i < length; ++i) {
            if (cmdline[i] == '\0') {
                cmdline[i] = ' ';
            }
        }
    } else {
        perror("Failed to read process command line");
        free(cmdline);
        cmdline = NULL;  // Return NULL to indicate an error
    }
    fclose(cmdline_file);

    return cmdline;
}

void saveFireFoxProfile(char* cmdLine) {
    regex_t regex;
    int ret;

    // Compile the regular expression
    ret = regcomp(&regex, "^/snap/firefox.*?/firefox( -P (\\w+))?", REG_EXTENDED);
    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        return;
    }

    // Execute the regular expression
    regmatch_t matches[3];
    if(regexec(&regex, cmdLine, 3, matches, 0) == 0) {
        char profileName[256];
        int profileNameLength = matches[2].rm_eo - matches[2].rm_so;
        strncpy(profileName, cmdLine + matches[2].rm_so, profileNameLength);
        profileName[profileNameLength] = '\0';

        const char *filename = "output.txt";
        FILE *file = fopen(filename, "w");
        if (!file) {
            perror("Failed to open file for writing");
        } else {
            // Write the byte array to the file
            size_t elementsWritten = fwrite(profileName, sizeof(char), profileNameLength, file);
            if (elementsWritten != profileNameLength) {
                perror("Failed to write to file");
                fclose(file);
            }
            fclose(file);
        }
    }

    // Free the compiled regular expression
    regfree(&regex);
    return;
}

int main() {
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        return 1;
    }

    XSelectInput(display, DefaultRootWindow(display), PropertyChangeMask);

    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.xproperty.atom == XInternAtom(display, "_NET_ACTIVE_WINDOW", False)) {
            Window active_window;
            int revert_to;
            XGetInputFocus(display, &active_window, &revert_to);

            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            unsigned char *prop;
            XGetWindowProperty(display, active_window, XInternAtom(display, "_NET_WM_PID", False),
                               0, 1, False, XA_CARDINAL, &actual_type, &actual_format,
                               &nitems, &bytes_after, &prop);

            if (actual_type == XA_CARDINAL && actual_format == 32) {
                char *cmdline = getPidCommand(*(int *)prop);
                printf("Command line for PID %s\n", cmdline);
                saveFireFoxProfile(cmdline);
                free(cmdline);
            }
            XFree(prop);
        }
    }

    XCloseDisplay(display);

    return 0;
}