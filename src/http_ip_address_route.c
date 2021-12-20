//
// Created by ghenry on 17/12/2021.
//

/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 Gavin Henry <ghenry@sentrypeer.org> */
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

#include <microhttpd.h>
#include <jansson.h>
#include "config.h"

#include "http_common.h"
#include "bad_actor.h"
#include "database.h"

int ip_address_route(char *ip_address, struct MHD_Connection *connection,
		     sentrypeer_config *config)
{
	const char *reply = 0;
	bad_actor *bad_actor_found = 0;

	if (db_select_bad_actor_by_ip(ip_address, &bad_actor_found, config) !=
	    EXIT_SUCCESS) {
		fprintf(stderr, "Failed to select bad actors from database\n");
		return MHD_NO;
	}

	if (bad_actor_found != 0) {
		if (config->verbose_mode || config->debug_mode) {
			fprintf(stderr, "bad_actor ip found: %s\n",
				bad_actor_found->source_ip);
		}

		json_t *json_final_obj = json_pack("{s:s}", "ip_address",
						   bad_actor_found->source_ip);
		reply = json_dumps(json_final_obj, JSON_INDENT(2));

		// Free the json objects
		json_decref(json_final_obj);
		bad_actor_destroy(&bad_actor_found);

		return finalise_response(connection, reply, CONTENT_TYPE_JSON,
					 MHD_HTTP_OK);
	} else {
		json_t *json_no_data =
			json_pack("{s:s}", "message", "No bad actor found");
		reply = json_dumps(json_no_data, JSON_INDENT(2));

		// Free the json object
		json_decref(json_no_data);
		bad_actor_destroy(&bad_actor_found);

		return finalise_response(connection, reply, CONTENT_TYPE_JSON,
					 MHD_HTTP_NOT_FOUND);
	}
}
