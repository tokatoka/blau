#ifndef BLAU_ELF_H
#define BLAU_ELF_H

#define EI_NIDENT 12
#define ELFMAGIC 0x464C457F
struct ELF{
        unsigned int    magic;
        unsigned char   e_ident[EI_NIDENT];
        unsigned short      e_type;
        unsigned short      e_machine;
        unsigned int      e_version;
        void *      e_entry;
        unsigned int       e_phoff;
        unsigned int       e_shoff;
        unsigned int      e_flags;
        unsigned short      e_ehsize;
        unsigned short      e_phentsize;
        unsigned short      e_phnum;
        unsigned short      e_shentsize;
        unsigned short      e_shnum;
        unsigned short      e_shstrndx;
};

struct Phdr {
        unsigned int      p_type;
        unsigned int       p_offset;
        void *      p_vaddr;
        void *      p_paddr;
        unsigned int      p_filesz;
        unsigned int      p_memsz;
        unsigned int      p_flags;
        unsigned int      p_align;
};


#endif