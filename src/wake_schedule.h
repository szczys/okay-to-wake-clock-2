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

struct otw_week {
    struct otw_day day[7];
    uint32_t crc;
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

int parse_schedule(struct otw_week *w, const char *payload, uint16_t len);
int ingest_schedule(const char *payload, uint16_t len);
void use_default_week(struct otw_week *sched);
void print_schedule_struct(struct otw_week *w);
void print_schedule(void);
int sched_to_big_time(int day, enum sched_events ev);
const char *get_event_str(uint8_t idx);
void otw_init(void);
void load_from_eeprom(struct otw_week *w);