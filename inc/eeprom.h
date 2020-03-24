#ifndef __EEPROM_H__
#define __EEPROM_H__

extern void vEEPROMconfig(void);
extern bool write_eeprom(uint32_t *pdata, uint32_t add_epprom, uint32_t count);
extern bool read_eeprom(uint32_t *pBuff, uint32_t add_epprom, uint32_t count);

#endif
