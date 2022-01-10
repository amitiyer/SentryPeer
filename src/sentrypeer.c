/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2022 Gavin Henry <ghenry@sentrypeer.org> */
/*
   _____            _              _____
  / ____|          | |            |  __ \
 | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
 |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
                              __/ |
                             |___/
*/

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <assert.h>

// Produced by autoconf and cmake (manually by me)
#include "config.h"

#include "conf.h"
#include "sip_daemon.h"
#include "http_daemon.h"

int main(int argc, char **argv)
{
	sentrypeer_config *config = sentrypeer_config_new();
	assert(config);

	if (process_cli(config, argc, argv) != EXIT_SUCCESS) {
		exit(EXIT_FAILURE);
	}

	if (config->syslog_mode) {
		openlog(PACKAGE_NAME, LOG_PID, LOG_USER);
	}

	if (config->debug_mode || config->verbose_mode) {
		fprintf(stderr, "Starting %s...\n", PACKAGE_NAME);
		if (config->syslog_mode) {
			syslog(LOG_ERR, "Starting %s...\n", PACKAGE_NAME);
		}
	}

	// Threaded, so start the HTTP daemon first
	if (config->api_mode) {
		if (http_daemon_init(config) == EXIT_FAILURE) {
			fprintf(stderr,
				"Failed to start %s server on port %d\n",
				"HTTP", HTTP_DAEMON_PORT);
			perror("http_daemon_init");
			if (config->syslog_mode) {
				syslog(LOG_ERR,
				       "Failed to start %s server on port %d\n",
				       "HTTP", HTTP_DAEMON_PORT);
			}
			exit(EXIT_FAILURE);
		}

		if (config->web_gui_mode &&
		    (config->debug_mode || config->verbose_mode)) {
			fprintf(stderr, "Web GUI mode enabled...\n");
		}
	}

	// Blocking, so start the SIP daemon last
	if (sip_daemon_init(config) == EXIT_FAILURE) {
		fprintf(stderr, "Failed to start %s server on port %s\n", "SIP",
			SIP_DAEMON_PORT);
		perror("sip_daemon_init");
		if (config->syslog_mode) {
			syslog(LOG_ERR,
			       "Failed to start %s server on port %d\n", "SIP",
			       HTTP_DAEMON_PORT);
		}
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
