#ifndef CONFIG_H
#define CONFIG_H
#if defined(PLAT_GB)
#include "config_gb.h"
#elif defined(PLAT_SMS)
#include "config_sms.h"
#else
#error "define PLAT_GB or PLAT_SMS"
#endif
#endif
