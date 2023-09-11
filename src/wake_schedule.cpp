#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <EEPROM.h>
#include <CRC32.h>
#include "wake_schedule.h"

char payload[] = "# Start with Monday\n# Format: blue, green, off, red\n# example: 0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0600|0615|0645|0700\n0615|0630|0700|1900\n0615|0630|0700|1900";

struct otw_week _otw_week_schedule;

const char *otw_event_str[E_MAX] = {
    DOZE_STR,
    WAKE_STR,
    DAY_STR,
    SLEEP_STR,
    UNKNOWN_STR
};

void write_week_to_eeprom(struct otw_week *w) {
	EEPROM.put(0, *w);
	EEPROM.commit();
}

uint32_t calc_week_crc(struct otw_week *w) {
	return CRC32::calculate((uint8_t *)&(w->dow), sizeof(w->dow));
}

void load_from_eeprom(struct otw_week *w) {
	EEPROM.begin(64);
	EEPROM.get(0, *w);
	uint32_t crc = calc_week_crc(w);

	if (crc != w->crc) {
		Serial.println("CRC mismatch, restoring default schedule in EEPROM");
		use_default_week(w);
		write_week_to_eeprom(w);
	} else {
		Serial.println("Loaded schedule from EEPROM");
	}
	print_schedule_struct(w);
}

void otw_init(void) {
	load_from_eeprom(&_otw_week_schedule);
}

void use_default_week(struct otw_week *sched) {
	for (uint8_t i = 0; i<7; i++) {
		sched->dow[i].doze.hour = DEFAULT_DOZE_H;
		sched->dow[i].doze.minute = DEFAULT_DOZE_M;
		sched->dow[i].wake.hour = DEFAULT_WAKE_H;
		sched->dow[i].wake.minute = DEFAULT_WAKE_M;
		sched->dow[i].day.hour = DEFAULT_OFF_H;
		sched->dow[i].day.minute = DEFAULT_OFF_M;
		sched->dow[i].sleep.hour = DEFAULT_SLEEP_H;
		sched->dow[i].sleep.minute = DEFAULT_SLEEP_M;
	}

	sched->crc = calc_week_crc(sched);
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

const char *weekdays[7] = { MON, TUE, WED, THU, FRI, SAT, SUN };

void print_schedule_struct(struct otw_week *w) {
    char msg_buf[64*7] = { 0 };
	for (uint8_t i = 0; i<7; i++) {
        uint16_t str_start = strlen(msg_buf);
		snprintf(msg_buf + str_start, sizeof(msg_buf)-str_start,
               "%s: Doze->%02d:%02d Wake->%02d:%02d Off->%02d:%02d Sleep->%02d:%02d\n",
		       weekdays[i],
		       w->dow[i].doze.hour, w->dow[i].doze.minute,
		       w->dow[i].wake.hour, w->dow[i].wake.minute,
		       w->dow[i].day.hour, w->dow[i].day.minute,
		       w->dow[i].sleep.hour, w->dow[i].sleep.minute
		       );
	}
    Serial.println(msg_buf);
}

void print_schedule(void) {
	print_schedule_struct(&_otw_week_schedule);
}

void set_event(struct otw_week *w, uint8_t dow, uint8_t event, uint8_t hour, uint8_t minute) {
	switch(event) {
		case 0:
			w->dow[dow].doze.hour = hour;
			w->dow[dow].doze.minute = minute;
			break;

		case 1:
			w->dow[dow].wake.hour = hour;
			w->dow[dow].wake.minute = minute;
			break;

		case 2:
			w->dow[dow].day.hour = hour;
			w->dow[dow].day.minute = minute;
			break;

		case 3:
			w->dow[dow].sleep.hour = hour;
			w->dow[dow].sleep.minute = minute;
			break;

		default:
			return;
	}
}

int parse_schedule(struct otw_week *w, const char *payload, uint16_t len) {
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
				set_event(w, line, event, decoded_hours, decoded_minutes);
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

    if (line != 7) {
        char sbuf[64];
        snprintf(sbuf, sizeof(sbuf), "Error processing scheule. Expected 7 lines but got %d.", line);
        Serial.println(sbuf);
        return -1;
    }

	Serial.println("Successfully processed 7 lines");
	return 0;
}

int ingest_schedule(const char *payload, uint16_t len)
{
	struct otw_week new_week;
	int err = parse_schedule(&new_week, payload, len);
	
	if (err) {
		return err;
	}
		
	new_week.crc = calc_week_crc(&new_week);
	if (new_week.crc != _otw_week_schedule.crc) {
		Serial.println("Saving new schedule to EEPROM");
		write_week_to_eeprom(&new_week);	
		load_from_eeprom(&_otw_week_schedule);
	} else {
		Serial.println("Received schedule matches stored schedule.");
	}

	return 0;
}

int sched_to_big_time(int dow, enum sched_events ev)
{
  switch(ev) {
    case E_DOZE:
        return (_otw_week_schedule.dow[dow].doze.hour*60) + _otw_week_schedule.dow[dow].doze.minute;
        break;

    case E_WAKE:
        return (_otw_week_schedule.dow[dow].wake.hour*60) + _otw_week_schedule.dow[dow].wake.minute;
        break;

    case E_DAY:
        return (_otw_week_schedule.dow[dow].day.hour*60) + _otw_week_schedule.dow[dow].day.minute;
        break;

    case E_SLEEP:
        return (_otw_week_schedule.dow[dow].sleep.hour*60) + _otw_week_schedule.dow[dow].sleep.minute;
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
