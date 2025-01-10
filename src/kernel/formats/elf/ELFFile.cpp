//
// Created by Pawel Reich on 12/22/24.
//

#include "ELFFile.hpp"

#include <stddef.h>
#include <common/status.h>

extern "C" {
#include "memory/memory.h"
#include <task/task.h>
#include <memory/heap/kheap.h>
}

ELFFile::ELFFile(void *data, size_t size) {
    m_data = data;
    m_data_sz = size;
}

ELFFile::~ELFFile() {
    kfree(m_data);
}

void * ELFFile::get_entry() const {
    return (void*) get_header()->e_entry;
}

bool ELFFile::is_valid() const {
    if (m_data == nullptr)
        return false;

    if (m_data_sz < 4)
        return false;

    auto* const header = static_cast<Elf32_Header *>(m_data);

    if (memcmp(header->e_ident,(void*) ELF_SIGNATURE, sizeof(ELF_SIGNATURE)) != 0)
        return false;

    if (header->e_ident[EI_CLASS] != ELFCLASS32)
        return false;

    if (header->e_ident[EI_DATA] != ELFDATA2LSB)
        return false;

    if (header->e_type != ET_EXEC)
        return false;

    if (header->e_entry != PROGRAM_VIRTUAL_ADDRESS)
        return false;

    if (header->e_phoff == 0) // Check if ELF File has program header.
        return false;

    return true;
}

ELFFile::Elf32_Header* ELFFile::get_header() const {
    return static_cast<Elf32_Header *>(m_data);
}

ELFFile::Elf32_Phdr* ELFFile::get_program_header() const {
    auto const header = get_header();
    if (header->e_phoff == 0)
        return nullptr;
    return reinterpret_cast<Elf32_Phdr *>(reinterpret_cast<int>(header) + header->e_phoff);
}

ELFFile::Elf32_Phdr* ELFFile::get_program_header(unsigned const int index) const {
    return &get_program_header()[index];
}

ELFFile::Elf32_Shdr* ELFFile::get_section_header() const {
    auto const header = get_header();
    return reinterpret_cast<Elf32_Shdr *>(reinterpret_cast<int>(header) + header->e_shoff);
}

ELFFile::Elf32_Shdr* ELFFile::get_section_header(unsigned const int index) const {
    return &get_section_header()[index];
}

const char* ELFFile::get_string_table() const {
    auto const header = get_header();
    return reinterpret_cast<const char *>(reinterpret_cast<int>(header) + get_section_header(header->e_shstrndx)->sh_offset);
}

int ELFFile::parse() const {

    if (!is_valid())
        return -EINFORMAT;

    return 0;
}