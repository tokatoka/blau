#ifndef BLAU_MULTIBOOT_H
#define BLAU_MULTIBOOT_H
#define MULTIBOOT_BOOTLOADER_MAGIC          0x2BADB002


typedef struct multiboot_info {
  unsigned long flags;
  unsigned long mem_lower;
  unsigned long mem_upper;
  unsigned long boot_device;
  unsigned long cmdline;
  unsigned long mods_count;
  unsigned long mods_addr;

  unsigned long pad0;
  unsigned long pad1;
  unsigned long pad2;
  unsigned long pad3;

  unsigned long mmap_length;
  unsigned long mmap_addr;
} __attribute__((__packed__)) multiboot_info;


typedef struct memory_map {
  unsigned long size;
  unsigned long base_addr_low;
  unsigned long base_addr_high;
  unsigned long length_low;
  unsigned long length_high;
  unsigned long type;
} __attribute__((__packed__)) memory_map;

#endif