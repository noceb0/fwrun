#ifndef COMPAT3DS_H
#define COMPAT3DS_H

#include <nds/ndstypes.h>

#define LANG_SUPPORT_IQUE 0x7E
#define LANG_SUPPORT_KOREAN 0xAF

#define MAGIC_VALID 0xFC

typedef enum {
    LANG_JAPANESE,
    LANG_ENGLISH,
    LANG_FRENCH,
    LANG_GERMAN,
    LANG_ITALIAN,
    LANG_SPANISH,
    LANG_CHINESE,
    LANG_KOREAN
} fwExtendedLanguage;

typedef struct {
    u8 present;
    u8 language;
    u16 supported_languages;
    u8 dontcare[0x86];
    u16 crc;
} fwPreferencesExtended;

// not using libnds struct because it lacks the ique/korean-specific extended settings
typedef struct {
    u16 version;
    u8 color;
    u8 bday_month;
    u8 bday_day;
    u8 dontcare0;
    u16 nickname[10];
    u16 nickname_length;
    u16 message[26];
    u16 message_length;
    u8 alarm_hour;
    u8 alarm_minute;
    u32 dontcare1;
    u8 calibration[0xC];
    u8 flags;
    u8 valid;
    u8 year;
    u8 dontcare3;
    u32 rtc_offset;
    u32 dontcare4;
    u16 update_counter;
    u16 crc;
    fwPreferencesExtended extended;
} fwPreferences;

void patch_preferences();

#endif