#ifndef VBA_REG_H
#define VBA_REG_H

extern bool regEnabled;

const char *regQueryStringValue(const char *key, const char *def);
DWORD regQueryDwordValue(const char *key, DWORD def, bool force = false);
BOOL regQueryBinaryValue(const char *key, char *value, int count);
void regSetStringValue(const char *key, const char *value);
void regSetDwordValue(const char *key, DWORD value, bool force = false);
void regSetBinaryValue(const char *key, char *value, int count);
void regDeleteValue(const char *key);
void regInit(const char *);
void regShutdown();
bool regCreateFileType(const char *ext, const char *type);
bool regAssociateType(const char *type, const char *desc, const char *application);
void regExportSettingsToINI();
#endif // VBA_REG_H
