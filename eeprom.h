#ifndef EEPROM_H_
#define EEPROM_H_

uint32_t eeprominit();
uint32_t readeeprom(uint32_t block, uint32_t offset);
void writeeeprom(uint32_t block, uint32_t offset, uint32_t data);
void eraseeeprom(uint32_t block, uint32_t offset);

#endif

