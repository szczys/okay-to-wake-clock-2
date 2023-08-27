#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "wake_schedule.h"

char payload[] = "# Start with Monday\n# Format: blue, green, off, red\n# example: 0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0615|0630|0700|1900\n0615|0630|0700|1900";

struct otw_day otw_week_schedule[7];

const char *otw_event_str[E_MAX] = {
    DOZE_STR,
    WAKE_STR,
    DAY_STR,
    SLEEP_STR,
    UNKNOWN_STR
};

void use_default_week(void) {
	for (uint8_t i = 0; i<7; i++) {
		otw_week_schedule[i].doze.hour = DEFAULT_DOZE_H;
		otw_week_schedule[i].doze.minute = DEFAULT_DOZE_M;
		otw_week_schedule[i].wake.hour = DEFAULT_WAKE_H;
		otw_week_schedule[i].wake.minute = DEFAULT_WAKE_M;
		otw_week_schedule[i].day.hour = DEFAULT_OFF_H;
		otw_week_schedule[i].day.minute = DEFAULT_OFF_M;
		otw_week_schedule[i].sleep.hour = DEFAULT_SLEEP_H;
		otw_week_schedule[i].sleep.minute = DEFAULT_SLEEP_M;
	}
}

int validate_time(char tens, char ones, uint8_t max) {
	int t_int = ((int)tens)-0x30;
	int m_int = ((int)ones)-0x30;

	if ((t_int < 0) || (m_int > 9)) { return -1; }
	if ((m_int < 0) || (m_int > 9)) { return -1; }

	int value = (t_int * 10) + m_int;
	
	if ((value < 0) || (value > max)) { return -1; }
	return value;
}

#define MON "Mon"
#define TUE "Tue"
#define WED "Wed"
#define THU "Thu"
#define FRI "Fri"
#define SAT "Sat"
#define SUN "Sun"

char *weekdays[7] = { MON, TUE, WED, THU, FRI, SAT, SUN };

void print_schedule(void) {
    char msg_buf[64*7] = { 0 };
	for (uint8_t i = 0; i<7; i++) {
        uint16_t str_start = strlen(msg_buf);
		snprintf(msg_buf + str_start, sizeof(msg_buf)-str_start,
               "%s: Doze->%02d:%02d Wake->%02d:%02d Off->%02d:%02d Sleep->%02d:%02d\n",
		       weekdays[i],
		       otw_week_schedule[i].doze.hour, otw_week_schedule[i].doze.minute,
		       otw_week_schedule[i].wake.hour, otw_week_schedule[i].wake.minute,
		       otw_week_schedule[i].day.hour, otw_week_schedule[i].day.minute,
		       otw_week_schedule[i].sleep.hour, otw_week_schedule[i].sleep.minute
		       );
	}
    my_log(msg_buf);
}

void set_event(uint8_t day, uint8_t event, uint8_t hour, uint8_t minute) {
	switch(event) {
		case 0:
			otw_week_schedule[day].doze.hour = hour;
			otw_week_schedule[day].doze.minute = minute;
			break;

		case 1:
			otw_week_schedule[day].wake.hour = hour;
			otw_week_schedule[day].wake.minute = minute;
			break;

		case 2:
			otw_week_schedule[day].day.hour = hour;
			otw_week_schedule[day].day.minute = minute;
			break;

		case 3:
			otw_week_schedule[day].sleep.hour = hour;
			otw_week_schedule[day].sleep.minute = minute;
			break;

		default:
			return;
	}
}

int parse_schedule(const char *payload, uint16_t len) {
	int16_t i = -1;

	uint8_t line = 0;
	uint8_t event = 0;
	uint8_t time_idx = 0;
	bool await_nl = false;

	int8_t decoded_hours = 0;
	int8_t decoded_minutes = 0;

	while(true) {
		i += 1;
		if (i >= len) {
			break;
		}

		// Currently parsing comment
		if (await_nl) {
			if (payload[i] == '\n') {
				await_nl = false;
			}
			continue;
		}

		// Filter for comment (starts with #)
		if (payload[i] == '#') {
			await_nl = true;
			continue;
		}

		switch(time_idx) {
			case 1:
				decoded_hours = validate_time(payload[i-1], payload[i], 23);
				if (decoded_hours < 0) {
					return -1;
				}
				time_idx++;
				break;
			case 3:
				decoded_minutes = validate_time(payload[i-1], payload[i], 59);
				if (decoded_minutes < 0) {
					return -1;
				}
				set_event(line, event, decoded_hours, decoded_minutes);
				if (event == 3) {
					line++;
					event = 0;
					time_idx = 0;
					await_nl = true;
				} else {
					time_idx++;
				}
				break;
			case 4:
				if (payload[i] == '|') {
					time_idx = 0;
					event++;
				} else {
					return -1;
				}
				break;
			default:
				time_idx++;
				break;
		}
        
	}

    if (line == 7) {
        my_log("Successfully processed 7 lines");
    } else {
        char sbuf[64];
        snprintf(sbuf, sizeof(sbuf), "Error processing scheule. Expected 7 lines but got %d.", line);
        my_log(sbuf);
        return -1;
    }
	return 0;
}

int sched_to_big_time(int day, enum sched_events ev)
{
  switch(ev) {
    case E_DOZE:
        return (otw_week_schedule[day].doze.hour*60) + otw_week_schedule[day].doze.minute;
        break;

    case E_WAKE:
        return (otw_week_schedule[day].wake.hour*60) + otw_week_schedule[day].wake.minute;
        break;

    case E_DAY:
        return (otw_week_schedule[day].day.hour*60) + otw_week_schedule[day].day.minute;
        break;

    case E_SLEEP:
        return (otw_week_schedule[day].sleep.hour*60) + otw_week_schedule[day].sleep.minute;
        break;
    default:
        //FIXME: This should never happen, but if it does it's an unhandled exception
        return -1;
  };
  return 0;
}

const char *get_event_str(uint8_t idx) {
    if (idx >= E_MAX) {
        return otw_event_str[E_UNKNOWN];
    }

    return otw_event_str[idx];
}