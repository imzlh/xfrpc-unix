
// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (c) 2023 Dengfeng Liu <liudf0716@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "config.h"
#include "commandline.h"
#include "debug.h"
#include "version.h"
#include "utils.h"

/* Type definitions */
typedef void signal_func(int);

/* Function declarations */
// static signal_func *set_signal_handler(int signo, signal_func *func);
static void usage(const char *appname);

/* Global configuration variables */
static struct {
    char *config_file;
} g_config = {
    .config_file = NULL
};

/* Accessor macros/inline functions */
#define IS_DAEMON() (g_config.is_daemon)
#define GET_CONFIG_FILE() (g_config.config_file)

/*
 * Sets up a signal handler for a specified signal number.
 *
 * @param signo  The signal number to handle
 * @param func   Pointer to the signal handling function
 * @return       Previous signal handler, or SIG_ERR on error
 */
// static signal_func *set_signal_handler(int signo, signal_func *func) {
// #ifndef __MSYS__
//     struct sigaction new_action, old_action;

//     /* Initialize the new signal action */
//     new_action.sa_handler = func;
//     sigemptyset(&new_action.sa_mask);
//     new_action.sa_flags = SA_RESTART;  /* Linux default behavior */

//     /* Install the signal handler */
//     if (sigaction(signo, &new_action, &old_action) < 0) {
//         return SIG_ERR;
//     }

//     return old_action.sa_handler;
// #else
//     return signal(signo, func);
// #endif
// }

int 
get_daemon_status()
{
    return 0;
}

/**
 * Displays program usage information
 *
 * Prints a formatted help message showing all available command-line options
 * and their descriptions.
 *
 * @param appname Name of the application executable
 */
static void usage(const char *appname)
{
    static const struct {
        const char *option;
        const char *description;
    } options[] = {
        {"-c [filename]", "Specify config file to use"},
        {"-f",           "Run in foreground (don't daemonize)"},
        {"-d <level>",   "Set debug level"},
        {"-s",           "Enable syslog for debug logging"},
        {"-h",           "Display this help message"},
        {"-v",           "Display version information"},
        {"-r",           "Display client run ID"}
    };

    fprintf(stdout, "Usage: %s [options]\n\n", appname);
    fprintf(stdout, "Options:\n");

    for (size_t i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
        fprintf(stdout, "  %-14s %s\n", 
                options[i].option, 
                options[i].description);
    }
    
    fprintf(stdout, "\n");
}

/**
 * Parses command line arguments and initializes configuration
 *
 * @param argc  Number of command line arguments
 * @param argv  Array of command line argument strings
 */
void parse_commandline(int argc, char **argv)
{
    int c;
    int config_specified = 0;

    while (-1 != (c = getopt(argc, argv, "c:hfd:svr"))) {
        switch (c) {
            case 'h':
                usage(argv[0]);
                exit(0);
                break;

            case 'c':
                if (optarg) {
                    g_config.config_file = strdup(optarg);
                    if (!g_config.config_file) {
                        debug(LOG_ERR, "Failed to allocate memory for config file path");
                        exit(1);
                    }
                    config_specified = 1;
                }
                break;

            case 'f':
                debugconf.log_stderr = 1;
                break;

            case 'd':
                if (optarg) {
                    debugconf.debuglevel = atoi(optarg);
                }
                break;
                
            case 's':
                debugconf.log_syslog = 1;
                break;

            case 'v':
                fprintf(stdout, "version: " VERSION "\n");
                exit(0);
                break;

            case 'r':
                {
                    char ifname[16] = {0};
                    char if_mac[64] = {0};

                    if (get_net_ifname(ifname, sizeof(ifname)) != 0) {
                        debug(LOG_ERR, "Failed to get network interface name");
                        exit(1);
                    }

                    if (get_net_mac(ifname, if_mac, sizeof(if_mac)) != 0) {
                        debug(LOG_ERR, "Failed to get MAC address for interface %s", ifname);
                        exit(1);
                    }

                    fprintf(stdout, "run ID: %s\n", if_mac);
                    exit(0);
                }
                break;

            default:
                usage(argv[0]);
                exit(1);
        }
    }

    if (!config_specified) {
        fprintf(stderr, "Error: Config file must be specified with -c option\n");
        usage(argv[0]);
        exit(1);
    }

    load_config(g_config.config_file);
}
