#pragma once

#include <stdint.h>

#define SCHEDULE_SERVER_PATH "https://192.168.1.105/download/okay_to_wake.txt"

struct otw_time {
	uint8_t hour;
	uint8_t minute;
};

struct otw_day {
	struct otw_time doze;
	struct otw_time wake;
	struct otw_time day;
	struct otw_time sleep;
};

#define DEFAULT_DOZE_H 6
#define DEFAULT_DOZE_M 15
#define DEFAULT_WAKE_H 6
#define DEFAULT_WAKE_M 30
#define DEFAULT_OFF_H 7
#define DEFAULT_OFF_M 0
#define DEFAULT_SLEEP_H 18
#define DEFAULT_SLEEP_M 45

enum sched_events { 
    E_DOZE,
    E_WAKE,
    E_DAY,
    E_SLEEP,
    E_UNKNOWN,
    E_MAX
};

#define DOZE_STR "Doze"
#define WAKE_STR "Wake"
#define DAY_STR "Day"
#define SLEEP_STR "Sleep"
#define UNKNOWN_STR "State Out-of-Bounds"

int parse_schedule(const char *payload, uint16_t len);
void use_default_week(void);
void print_schedule(void);
int sched_to_big_time(int day, enum sched_events);
const char *get_event_str(uint8_t idx);
