/**
 * @file datapipe.h
 * Headers for the simple filter framework
 * <p>
 * Copyright © 2007 Nokia Corporation and/or its subsidiary(-ies).
 * <p>
 * @author David Weinehall <david.weinehall@nokia.com>
 *
 * mce is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * mce is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with mce.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _DATAPIPE_H_
#define _DATAPIPE_H_

#include <stdbool.h>
#include <glib.h>

/** Device lock states used in device_lock_state_pipe */
typedef enum
{
    /** Device lock is not active */
    DEVICE_LOCK_UNLOCKED  = 0,

    /** Device lock is active */
    DEVICE_LOCK_LOCKED    = 1,

    /** Initial startup value; from mce p.o.v. equals not active */
    DEVICE_LOCK_UNDEFINED = 2,
}  device_lock_state_t;

const char *device_lock_state_repr(device_lock_state_t state);

/**
 * Datapipe structure
 *
 * Only access this struct through the functions
 */
typedef struct {
	GSList *filters;		/**< The filters */
	GSList *input_triggers;		/**< Triggers called on indata */
	GSList *output_triggers;	/**< Triggers called on outdata */
	gpointer cached_data;		/**< Latest cached data */
	gsize datasize;			/**< Size of data; NULL == automagic */
	gboolean free_cache;		/**< Free the cache? */
	gboolean read_only;		/**< Datapipe is read only */
} datapipe_struct;

/**
 * Read only policy type
 */
typedef enum {
	READ_WRITE = FALSE,		/**< The pipe is read/write */
	READ_ONLY = TRUE		/**< The pipe is read only */
} read_only_policy_t;

/**
 * Policy used for the cache when freeing a datapipe
 */
typedef enum {
	DONT_FREE_CACHE = FALSE,	/**< Don't free the cache */
	FREE_CACHE = TRUE		/**< Free the cache */
} cache_free_policy_t;

/**
 * Policy for the data source
 */
typedef enum {
	USE_INDATA = FALSE,		/**< Use the indata as data source */
	USE_CACHE = TRUE		/**< Use the cache as data source */
} data_source_t;

/**
 * Policy used for caching indata
 */
typedef enum {
	DONT_CACHE_INDATA = 0,          /**< Do not cache the indata */
	CACHE_INDATA      = 1<<0,       /**< Cache the unfiltered indata */
	CACHE_OUTDATA     = 1<<1,       /**< Cache the filtered outdata */
} caching_policy_t;

/* Available datapipes */
extern datapipe_struct led_brightness_pipe;
extern datapipe_struct lpm_brightness_pipe;
extern datapipe_struct device_inactive_state_pipe;
extern datapipe_struct device_inactive_event_pipe;
extern datapipe_struct led_pattern_activate_pipe;
extern datapipe_struct led_pattern_deactivate_pipe;
extern datapipe_struct device_resumed_pipe;
extern datapipe_struct user_activity_pipe;
extern datapipe_struct display_state_pipe;
extern datapipe_struct display_state_req_pipe;
extern datapipe_struct display_state_next_pipe;
extern datapipe_struct exception_state_pipe;
extern datapipe_struct display_brightness_pipe;
extern datapipe_struct key_backlight_pipe;
extern datapipe_struct keypress_pipe;
extern datapipe_struct touchscreen_pipe;
extern datapipe_struct lockkey_pipe;
extern datapipe_struct keyboard_slide_pipe;
extern datapipe_struct keyboard_available_pipe;
extern datapipe_struct lid_sensor_is_working_pipe;
extern datapipe_struct lid_cover_sensor_pipe;
extern datapipe_struct lid_cover_policy_pipe;
extern datapipe_struct lens_cover_pipe;
extern datapipe_struct proximity_sensor_pipe;
extern datapipe_struct ambient_light_sensor_pipe;
extern datapipe_struct ambient_light_level_pipe;
extern datapipe_struct ambient_light_poll_pipe;
extern datapipe_struct orientation_sensor_pipe;
extern datapipe_struct alarm_ui_state_pipe;
extern datapipe_struct system_state_pipe;
extern datapipe_struct master_radio_pipe;
extern datapipe_struct submode_pipe;
extern datapipe_struct call_state_pipe;
extern datapipe_struct ignore_incoming_call_pipe;
extern datapipe_struct call_type_pipe;
extern datapipe_struct tk_lock_pipe;
extern datapipe_struct interaction_expected_pipe;
extern datapipe_struct charger_state_pipe;
extern datapipe_struct battery_status_pipe;
extern datapipe_struct battery_level_pipe;
extern datapipe_struct camera_button_pipe;
extern datapipe_struct inactivity_timeout_pipe;
extern datapipe_struct audio_route_pipe;
extern datapipe_struct usb_cable_pipe;
extern datapipe_struct jack_sense_pipe;
extern datapipe_struct power_saving_mode_pipe;
extern datapipe_struct thermal_state_pipe;
extern datapipe_struct heartbeat_pipe;
extern datapipe_struct compositor_available_pipe;
extern datapipe_struct lipstick_available_pipe;
extern datapipe_struct devicelock_available_pipe;
extern datapipe_struct usbmoded_available_pipe;
extern datapipe_struct ngfd_available_pipe;
extern datapipe_struct dsme_available_pipe;
extern datapipe_struct bluez_available_pipe;
extern datapipe_struct packagekit_locked_pipe;
extern datapipe_struct update_mode_pipe;
extern datapipe_struct shutting_down_pipe;
extern datapipe_struct device_lock_state_pipe;
extern datapipe_struct touch_detected_pipe;
extern datapipe_struct touch_grab_wanted_pipe;
extern datapipe_struct touch_grab_active_pipe;
extern datapipe_struct keypad_grab_wanted_pipe;
extern datapipe_struct keypad_grab_active_pipe;
extern datapipe_struct music_playback_pipe;
extern datapipe_struct proximity_blank_pipe;
extern datapipe_struct wristgesture_sensor_pipe;

/* Data retrieval */

/** Retrieve a gboolean from a datapipe */
#define datapipe_get_gbool(_datapipe)	(GPOINTER_TO_INT((_datapipe).cached_data))

/** Retrieve a gint from a datapipe */
#define datapipe_get_gint(_datapipe)	(GPOINTER_TO_INT((_datapipe).cached_data))

/** Retrieve a guint from a datapipe */
#define datapipe_get_guint(_datapipe)	(GPOINTER_TO_UINT((_datapipe).cached_data))

/** Retrieve a gsize from a datapipe */
#define datapipe_get_gsize(_datapipe)	(GPOINTER_TO_SIZE((_datapipe).cached_data))

/** Retrieve a gpointer from a datapipe */
#define datapipe_get_gpointer(_datapipe)	((_datapipe).cached_data)

/* Datapipe execution */
void execute_datapipe_input_triggers(datapipe_struct *const datapipe,
				     gpointer const indata,
				     const data_source_t use_cache);
gconstpointer execute_datapipe_filters(datapipe_struct *const datapipe,
				       gpointer indata,
				       const data_source_t use_cache);
void execute_datapipe_output_triggers(const datapipe_struct *const datapipe,
				      gconstpointer indata,
				      const data_source_t use_cache);
gconstpointer execute_datapipe(datapipe_struct *const datapipe,
			       gpointer indata,
			       const data_source_t use_cache,
			       const caching_policy_t cache_indata);

/* Filters */
void append_filter_to_datapipe(datapipe_struct *const datapipe,
			       gpointer (*filter)(gpointer data));
void remove_filter_from_datapipe(datapipe_struct *const datapipe,
				 gpointer (*filter)(gpointer data));

/* Input triggers */
void append_input_trigger_to_datapipe(datapipe_struct *const datapipe,
				      void (*trigger)(gconstpointer data));
void remove_input_trigger_from_datapipe(datapipe_struct *const datapipe,
					void (*trigger)(gconstpointer data));

/* Output triggers */
void append_output_trigger_to_datapipe(datapipe_struct *const datapipe,
				       void (*trigger)(gconstpointer data));
void remove_output_trigger_from_datapipe(datapipe_struct *const datapipe,
					 void (*trigger)(gconstpointer data));

void setup_datapipe(datapipe_struct *const datapipe,
		    const read_only_policy_t read_only,
		    const cache_free_policy_t free_cache,
		    const gsize datasize, gpointer initial_data);
void free_datapipe(datapipe_struct *const datapipe);

/* Binding arrays */

typedef struct
{
    datapipe_struct *datapipe;
    void (*output_cb)(gconstpointer data);
    void (*input_cb)(gconstpointer data);
    gpointer (*filter_cb)(gpointer data);
    bool bound;
} datapipe_handler_t;

void datapipe_handlers_install(datapipe_handler_t *bindings);
void datapipe_handlers_remove(datapipe_handler_t *bindings);
void datapipe_handlers_execute(datapipe_handler_t *bindings);

typedef struct
{
    const char         *module;
    datapipe_handler_t *handlers;
    guint               execute_id;
} datapipe_bindings_t;

void datapipe_bindings_init(datapipe_bindings_t *self);
void datapipe_bindings_quit(datapipe_bindings_t *self);

/* Startup / exit */
void mce_datapipe_init(void);
void mce_datapipe_quit(void);

/* Helper for making display state requests
 *
 * This needs to be macro so that logging context stays
 * at the point of call.
 */
#define mce_datapipe_req_display_state(state_) do {\
    display_state_t cur_target = datapipe_get_gint(display_state_next_pipe);\
    display_state_t req_target = (display_state_t)(state_);\
    /* Use elevated logginng verbosity for requests that \
     * are likely to result in display power up. */ \
    int level = LL_DEBUG; \
    if( cur_target != req_target ) {\
	switch( req_target ) {\
	case MCE_DISPLAY_ON:\
	case MCE_DISPLAY_LPM_ON:\
	    level = LL_CRUCIAL;\
	    break;\
	default:\
	    break;\
	}\
    }\
    mce_log(level, "display state req: %s",\
	    display_state_repr(req_target));\
    /* But the request must always be fed to the datapipe \
     * because during already ongoing transition something \
     * else might be already queued up and we want't the \
     * last request to reach the queue to "win". */ \
    execute_datapipe(&display_state_req_pipe,\
		     GINT_TO_POINTER(req_target),\
		     USE_INDATA, CACHE_OUTDATA);\
} while(0)

#endif /* _DATAPIPE_H_ */
