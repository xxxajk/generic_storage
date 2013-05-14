/*
 * (CopyLeft) 2013 Andrew J. Kroll
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <FAT/FAT.h>
#include <Storage.h>
#include <FAT/FatFS/src/diskio.h>

uint32_t faux_clock() {
        /*
        bit31:25 year 33
        bit24:21 month 1
        bit20:16 day 1
        bit15:11 h 0
        bit10:5 m 0
        bit4:0 s 0

        3322 2222 2222 1111 1111 11
        1098 7654 3210 9876 5432 1098 7654 3210
        YYYY YYYM MMMD DDDD HHHH HMMM MMMS SSSS
        7654 3210 7654 3210 7654 3210 7654 3210
        0110 0110 0010 0001 0000 0000 0000 0000
        8421 8421 8421 8421 8421 8421 8421 8421
        6    6    2    1    0    0    0    0
        2013/1/1 00:00:00
         */
        return 0x66210000UL;

}

PFAT::PFAT() {
        set_clock_call((void *)&faux_clock);
}

int PFAT::Init(storage_t *sto, uint8_t lv) {
        Init(sto, lv, (uint32_t)0);
}

/* Identify the FAT type. */
int PFAT::Init(storage_t *sto, uint8_t lv, uint32_t first) {
        uint8_t buf[sto->SectorSize];
        //buf = (uint8_t *)malloc(sto->SectorSize);
        st = (int)((sto->Read)(first, buf, sto));
        if (!st) {
                fat_boot_t *BR = (fat_boot_t *)buf;
                // verify that the sig is OK.
                if (BR->bootSectorSig0 != 0x55 || BR->bootSectorSig1 != 0xAA) {
                        printf("Bad sig? %02x %02x\r\n", BR->bootSectorSig0, BR->bootSectorSig1);
                        st = -1;
                } else {
                        Offset = first;
                        storage = sto;
                        ffs = new FATFS;
                        ffs->pfat = this;
                        volmap = lv;
                        st = f_mount(volmap, ffs);
                        if (!st) {
                                if(label == NULL) {
                                        label = (uint8_t *)malloc(256);
                                }
                                *label = 0x00;
                                TCHAR path[4];
                                path[0] = '0'+lv;
                                path[1]=':';
                                path[2]='/';
                                path[3]=0x00;
                                DWORD sn;
                                int t = f_getlabel(&path[0], (TCHAR *)label, &sn);
                                if(t) {
                                        printf("NO LABEL\r\n");
                                        *label = 0x00;
                                } else {
                                        printf("VOLUME %i @ '%s'\r\n", volmap, label);
                                        // We will need to convert 'wide' chars, etc-- yuck!
                                        // Life would be a whole lot easier if everything was just UTF-8!
                                }
                        } else {
                                printf("Mount failed %i(%x)\r\n", st, st);
                        }
                }
        }
        //free(buf);

        return st;
}

/*
    disk_initialize - Initialize disk drive (no need!)
    disk_status - Get disk status
    disk_read - Read sector(s)
    disk_write - Write sector(s)
    disk_ioctl - Control device dependent features
    get_fattime - Get current time
 */
DSTATUS PFAT::disk_initialize(BYTE pdrv) {
        return 0;
}

DSTATUS PFAT::disk_status(BYTE pdrv) {
        return 0; // TO-DO: write protect status.
}

DRESULT PFAT::disk_read(BYTE pdrv, BYTE *buff, DWORD sector, BYTE count) {

        storage->Reads(sector, (uint8_t*)buff, storage, count);
}

DRESULT PFAT::disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, BYTE count) {

        storage->Writes(sector, (uint8_t*)buff, storage, count);
}

DRESULT PFAT::disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
        switch (cmd) {
                case CTRL_SYNC:
                        break;
                case GET_SECTOR_COUNT:
                        *(DWORD*)buff = storage->TotalSectors;
                        break;
                case GET_BLOCK_SIZE:
                        *(DWORD*)buff = storage->SectorSize;

                        break;
                default:
                        return RES_PARERR;
        }
        return RES_OK;
}

uint32_t PFAT::get_fattime() {

        return ((clock_call)());
}

void PFAT::set_clock_call(void *call) {

        clock_call = (uint32_t(*)())call;
}

/* This function is defined for only project compatibility               */

void disk_timerproc(void) {
        /* Nothing to do */
}

int PFAT::ReadSector(uint32_t sector, uint8_t *buf) {

        return storage->Read(Offset + sector, buf, storage);
}

int PFAT::WriteSector(uint32_t sector, uint8_t *buf) {

        return storage->Write(Offset + sector, buf, storage);
}

int PFAT::ReadSectors(uint32_t sector, uint8_t *buf, uint8_t count) {

        return storage->Reads(Offset + sector, buf, storage, count);
}

int PFAT::WriteSectors(uint32_t sector, uint8_t *buf, uint8_t count) {

        return storage->Writes(Offset + sector, buf, storage, count);
}

/*
 * Stuff to-do yet (mount points)
 *

// translate path to volume number, *WARNING* modifies string in-place!
void PFAT::TranslateMount(uint8_t *path) {

}

// Test if this instance controls the path asked for
uint8_t PFAT::Mypath(uint8_t *path) {

}
                last_state = current_state;

 */
PFAT::~PFAT() {
        if (ffs != NULL) {
                f_mount(volmap, NULL);
                delete ffs;
                ffs = NULL;
        }

        if(label != NULL) {
                free(label);
                label = NULL;
        }
}
// Allow callbacks from C to C++ class methods.
extern "C" {

        DSTATUS CPP_PFAT_disk_initialize(PFAT * pfat, BYTE pdrv) {
                return pfat->disk_initialize(pdrv);
        }

        DSTATUS CPP_PFAT_disk_status(PFAT * pfat, BYTE pdrv) {

                return pfat->disk_status(pdrv);
        }

        DRESULT CPP_PFAT_disk_read(PFAT * pfat, BYTE pdrv, BYTE *buff, DWORD sector, BYTE count) {

                return pfat->disk_read(pdrv, buff, sector, count);
        }

        DRESULT CPP_PFAT_disk_write(PFAT * pfat, BYTE pdrv, const BYTE *buff, DWORD sector, BYTE count) {
                return pfat->disk_write(pdrv, buff, sector, count);
        }

        DRESULT CPP_PFAT_disk_ioctl(PFAT * pfat, BYTE pdrv, BYTE cmd, void* buff) {
                return pfat->disk_ioctl(pdrv, cmd, buff);
        }

        uint32_t CPP_PFAT_get_fattime(PFAT * pfat) {
                return pfat->get_fattime();
        }
}
