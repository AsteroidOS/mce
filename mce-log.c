/**
 * @file mce-log.c
 * Logging functions for Mode Control Entity
 * <p>
 * Copyright © 2006-2007, 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include <stdlib.h>
#include <stdbool.h>
#include <fnmatch.h>

#include <glib.h>
#include <glib/gprintf.h>

#ifdef OSSOLOG_COMPILE
#include <stdio.h>			/* fprintf() */
#include <stdarg.h>			/* va_start(), va_end(), vfprintf() */
#include <string.h>			/* strdup() */
#include <syslog.h>			/* openlog(), closelog(), vsyslog() */
#include <sys/time.h>
#include "mce-log.h"

static unsigned int logverbosity = LL_WARN;	/**< Log verbosity */
static int logtype = MCE_LOG_STDERR;		/**< Output for log messages */
static char *logname = NULL;

/** Get monotonic time as struct timeval */
static void monotime(struct timeval *tv)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	TIMESPEC_TO_TIMEVAL(tv, &ts);
}
static void timestamp(struct timeval *tv)
{
	static struct timeval start, prev;
	struct timeval diff;
	monotime(tv);
	timersub(tv, &prev, &diff);
	if( diff.tv_sec >= 4 ) {
	  timersub(tv, &start, &diff);
	  fprintf(stderr, "%s: T+%ld.%03ld %s\n\n",
		  logname,
		  (long)diff.tv_sec, (long)(diff.tv_usec/1000),
		  "END OF BURST");
	  start = *tv;
	}
	prev = *tv;
	timersub(tv, &start, tv);
}

/** Make sure loglevel is in the supported range
 *
 * @param loglevel level to check
 *
 * @return log level clipped to LL_CRIT ... LL_DEBUG range
 */
static loglevel_t mce_log_level_normalize(loglevel_t loglevel)
{
	if( loglevel < LL_ALERT ) {
		loglevel = LL_ALERT;
	}
	else if( loglevel > LL_DEBUG ) {
		loglevel = LL_DEBUG;
	}
	return loglevel;
}

/** Get level indication tag to include in stderr logging
 *
 * @param loglevel level for the message
 *
 * @return level indication string
 */
static const char *mce_log_level_tag(loglevel_t loglevel)
{
	const char *res = "?";
	switch( loglevel ) {
        case LL_ALERT:  res = "A"; break;
        case LL_CRIT:   res = "C"; break;
        case LL_ERR:    res = "E"; break;
        case LL_WARN:   res = "W"; break;
        case LL_NOTICE: res = "N"; break;
        case LL_INFO:   res = "I"; break;
        case LL_DEBUG:  res = "D"; break;
        default: break;
        }
	return res;
}

/**
 * Log debug message with optional filename and function name attached
 *
 * @param loglevel The level of severity for this message
 * @param fmt The format string for this message
 * @param ... Input to the format string
 */
void mce_log_file(loglevel_t loglevel, const char *const file,
		  const char *const function, const char *const fmt, ...)
{
	va_list args;

	loglevel = mce_log_level_normalize(loglevel);

	if( mce_log_p_(loglevel, file, function) ) {
		gchar *msg = 0;

		va_start(args, fmt);
		g_vasprintf(&msg, fmt, args);
		va_end(args);

		if( file && function ) {
			gchar *tmp = g_strconcat(file, ": ", function, "(): ",
						 msg, NULL);
			g_free(msg), msg = tmp;
		}

		if (logtype == MCE_LOG_STDERR) {
			struct timeval tv;
			timestamp(&tv);
			fprintf(stderr, "%s: T+%ld.%03ld %s: %s\n",
				logname,
				(long)tv.tv_sec, (long)(tv.tv_usec/1000),
				mce_log_level_tag(loglevel),
				msg);
		} else {
			/* loglevels are subset of syslog priorities, so
			 * we can use loglevel as is for syslog priority */
			syslog(loglevel, "%s", msg);
		}

		g_free(msg);
	}
}

/**
 * Set log verbosity
 * messages with loglevel higher than or equal to verbosity will be logged
 *
 * @param verbosity minimum level for log level
 */
void mce_log_set_verbosity(const int verbosity)
{
	logverbosity = verbosity;
}

/**
 * Open log
 *
 * @param name identifier to use for log messages
 * @param facility the log facility; normally LOG_USER or LOG_DAEMON
 * @param type log type to use; MCE_LOG_STDERR or MCE_LOG_SYSLOG
 */
void mce_log_open(const char *const name, const int facility, const int type)
{
	logtype = type;

	if (logtype == MCE_LOG_SYSLOG)
		openlog(name, LOG_PID | LOG_NDELAY, facility);
	else
		logname = g_strdup(name);
}

/**
 * Close log
 */
void mce_log_close(void)
{
	g_free(logname), logname = 0;

	if (logtype == MCE_LOG_SYSLOG)
		closelog();
}

static GSList     *mce_log_patterns = 0;
static GHashTable *mce_log_functions = 0;

void mce_log_add_pattern(const char *pat)
{
    // NB these are never released by desing

    mce_log_patterns = g_slist_prepend(mce_log_patterns, strdup(pat));

    if( !mce_log_functions )
	mce_log_functions = g_hash_table_new_full(g_str_hash, g_str_equal,
						  free, 0);
}

static bool mce_log_check_pattern(const char *func)
{
    gpointer hit = 0;

    if( !mce_log_functions )
	goto EXIT;

    if( (hit = g_hash_table_lookup(mce_log_functions, func)) )
	goto EXIT;

    hit = GINT_TO_POINTER(1);

    for( GSList *item = mce_log_patterns; item; item = item->next ) {
	const char *pat = item->data;
	if( fnmatch(pat, func, 0) != 0 )
	    continue;
	hit = GINT_TO_POINTER(2);
	break;
    }
    g_hash_table_replace(mce_log_functions, strdup(func), hit);

EXIT:
    return GPOINTER_TO_INT(hit) > 1;
}

/**
 * Log level testing predicate
 *
 * For testing whether given level of logging is allowed
 * before spending cpu time for gathering parameters etc
 *
 * @param loglevel level of logging we might do
 *
 * @return 1 if logging at givel level is enabled, 0 if not
 */
int mce_log_p_(const loglevel_t loglevel,
	       const char *const file,
	       const char *const func)
{
    if( mce_log_functions && file && func ) {
	char temp[256];
	snprintf(temp, sizeof temp, "%s:%s", file, func);
	if( mce_log_check_pattern(temp) )
	    return true;
    }

    if( loglevel < LL_CRIT )
	return logverbosity >= LL_NOTICE;

    return logverbosity >= loglevel;
}

#endif /* OSSOLOG_COMPILE */
