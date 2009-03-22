#ifndef CONFIG_H
#define CONFIG_H

int config_get_int(const char *name);
const char *config_get_str(const char *str);
int config_get_int_from_table(const char *table, const char *name);
const char *config_get_str_from_table(const char *table, const char *name);

#endif
