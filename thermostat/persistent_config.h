#ifndef PERSISTENT_CONFIG_H
#define PERSISTENT_CONFIG_H

#include "FS.h"
#include "aJSON.h"

#include "thermostat.h"
#include "configuration.h"

void setup_config(thermoCfg *);
bool loadConfig(thermoCfg *);
void writeConfig(thermoCfg *);
void dump_cfg_file();

#endif
