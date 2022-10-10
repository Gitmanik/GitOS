; FAT 16 Header
OEMIdentifier        db 'GIT OS  '
BytesPerSector       dw 0x200 ; 512 bytes per sector
SectorsPerCluster    db 0x80
ReservedSectors      dw 200 ; TODO: Shrink it down and move kernel to filesystem
FATCopies            db 0x02
RootDirEntries       dw 0x40
NumSectors           dw 0x00
MediaType            db 0xF8
SectorsPerFat        dw 0x100
SectorsPerTrack      dw 0x20
NumberOfHeads        dw 0x40
HiddenSectors        dd 0x00
SectorsBig           dd 0x773594

; Extended BPB (FAT 16)
DriveNumber          db 0x80
WinNTBit             db 0x00
Signature            db 0x29
VolumeID             dd 0xD105
VolumeIDString       db 'GIT OS BOOT'
SystemIDString       db 'FAT16   '