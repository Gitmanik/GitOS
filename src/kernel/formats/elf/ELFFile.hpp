//
// Created by Pawel Reich on 12/22/24.
//
#pragma once

#include <stdint.h>
extern "C" {
#include "fs/pathparser.h"
}

class ELFFile {

  public:
    void* get_entry_ptr();
    bool is_valid() const;

  private:

    char filename[MAX_PATH];

    void* data;
    int data_sz;

    /**
    * Physical memory address of this binary.
    */
    void* phys_base_address;

    /**
     * Physical end memory adress of this binary.
     */
    void* phys_end_address;

    /**
    * Virtual memory address that this binary.
    */
    void* virt_base_address;

    /**
     * Virtual end memory address of this binary.
     */
    void* virt_end_address;

    static constexpr char ELF_SIGNATURE[] = {0x7f, 'E', 'L', 'F'};

    static const unsigned int PF_X = 1;
    static const unsigned int PF_R = 4;
    static const unsigned int PF_W = 2;

    static const unsigned int PT_NULL = 0;
    static const unsigned int PT_LOAD = 1;
    static const unsigned int PT_DYNAMIC = 2;
    static const unsigned int PT_INTERP = 3;
    static const unsigned int PT_NOTE = 4;
    static const unsigned int PT_SHLIB = 5;
    static const unsigned int PT_PHDR = 6;

    static const unsigned int SHT_NULL = 0;
    static const unsigned int SHT_PROGBITS = 1;
    static const unsigned int SHT_SYMTAB = 2;
    static const unsigned int SHT_STRTAB = 3;
    static const unsigned int SHT_RELA = 4;
    static const unsigned int SHT_HASH = 5;
    static const unsigned int SHT_DYNAMIC = 6;
    static const unsigned int SHT_NOTE = 7;
    static const unsigned int SHT_NOBITS = 8;
    static const unsigned int SHT_REL = 9;
    static const unsigned int SHT_SHLIB = 10;
    static const unsigned int SHT_DYNSYM = 11;
    static const unsigned int SHT_LOPROC = 12;
    static const unsigned int SHT_HIPROC = 13;
    static const unsigned int SHT_LOUSER = 14;
    static const unsigned int SHT_HIUSER = 15;

    static const unsigned int ET_NONE = 0;
    static const unsigned int ET_REL = 1;
    static const unsigned int ET_EXEC = 2;
    static const unsigned int ET_DYN = 3;
    static const unsigned int ET_CORE = 4;

    static const unsigned int EI_NIDENT = 16;
    static const unsigned int EI_CLASS = 4;
    static const unsigned int EI_DATA = 5;

    static const unsigned int ELFCLASSNONE = 0;
    static const unsigned int ELFCLASS32 = 1;
    static const unsigned int ELFCLASS64 = 2;

    static const unsigned int ELFDATANONE = 0;
    static const unsigned int ELFDATA2MSB = 1;
    static const unsigned int ELFDATA2LSB = 2;

    static const unsigned int SHN_UNDEF = 0;

    typedef uint16_t Elf32_Half;
    typedef uint32_t Elf32_Word;
    typedef int32_t Elf32_Sword;
    typedef uint32_t Elf32_Addr;
    typedef int32_t Elf32_Off;

    typedef struct __attribute__((__packed__)) {
        Elf32_Word p_type;
        Elf32_Off p_offset;
        Elf32_Addr p_vaddr;
        Elf32_Addr p_paddr;
        Elf32_Word p_filesz;
        Elf32_Word p_memsz;
        Elf32_Word p_flags;
        Elf32_Word p_align;
    } Elf32_Phdr;

    typedef struct __attribute__((__packed__)) {
        Elf32_Word sh_name;
        Elf32_Word sh_type;
        Elf32_Word sh_flags;
        Elf32_Addr sh_addr;
        Elf32_Off sh_offset;
        Elf32_Word sh_size;
        Elf32_Word sh_link;
        Elf32_Word sh_info;
        Elf32_Word sh_addralign;
        Elf32_Word sh_entsize;
    } Elf32_Shdr;

    typedef struct __attribute__((__packed__)) {
      unsigned char e_ident[EI_NIDENT];
      Elf32_Half e_type;
      Elf32_Half e_machine;
      Elf32_Word e_version;
      Elf32_Addr e_entry;
      Elf32_Off e_phoff;
      Elf32_Off e_shoff;
      Elf32_Word e_flags;
      Elf32_Half e_ehsize;
      Elf32_Half e_phentsize;
      Elf32_Half e_phnum;
      Elf32_Half e_shentsize;
      Elf32_Half e_shnum;
      Elf32_Half e_shstrndx;
    } Elf32_Header;

    typedef struct __attribute__((__packed__)) {
      Elf32_Sword d_tag;
      union {
        Elf32_Word d_val;
        Elf32_Addr d_ptr;
      } d_un;
    } Elf32_Dyn;

    typedef struct __attribute__((__packed__)) {
      Elf32_Word st_name;
      Elf32_Addr st_value;
      Elf32_Word st_size;
      unsigned char st_info;
      unsigned char st_other;
      Elf32_Half st_shndx;
    } Elf32_Sym;

    Elf32_Header *get_header() const;

    Elf32_Phdr *get_program_header() const;

    Elf32_Phdr *get_program_header(unsigned int index) const;

    Elf32_Shdr *get_section_header() const;

    Elf32_Shdr *get_section_header(unsigned int index) const;

    const char *get_string_table() const;
};