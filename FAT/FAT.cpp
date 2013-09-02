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
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <FAT/FAT.h>
#include <Storage.h>
#include <FAT/FatFS/src/diskio.h>
#include <Wire.h>
#include <RTClib.h>


#ifdef RTCLIB_H
RTC_DS1307 DS1307_RTC;
RTC_Millis ARDUINO_MILLIS_RTC;
#endif

#ifndef RTCLIB_H

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
#else

uint32_t RTClock() {
        if (DS1307_RTC.isrunning()) return DS1307_RTC.now().FatPacked();
        return ARDUINO_MILLIS_RTC.now().FatPacked();
}
#endif

static boolean WireStarted = false;

PFAT::PFAT() {
        label = NULL;
        ffs = NULL;
#ifndef RTCLIB_H
        set_clock_call((void *)&faux_clock);
#else
        if (!WireStarted) {
                WireStarted = true;
                Wire.begin();
                if (!DS1307_RTC.isrunning())
                        DS1307_RTC.adjust(DateTime(__DATE__, __TIME__));
                if (!DS1307_RTC.isrunning())
                        ARDUINO_MILLIS_RTC.begin(DateTime(__DATE__, __TIME__));
        }
        set_clock_call((void*)&RTClock);
#endif
}

int PFAT::Init(storage_t *sto, uint8_t lv) {
        Init(sto, lv, (uint32_t)0);
}

/* Identify the FAT type. */
int PFAT::Init(storage_t *sto, uint8_t lv, uint32_t first) {
        uint8_t buf[sto->SectorSize];
        TCHAR lb[256];
        lb[0] = 0x00;
        int i = 0;
        if (lv > _VOLUMES) return FR_INVALID_DRIVE;
        //buf = (uint8_t *)malloc(sto->SectorSize);
        st = (int)(sto->Read)(first, buf, sto);
        if (!st) {
                fat_boot_t *BR = (fat_boot_t *)buf;
                // verify that the sig is OK.
                if (BR->bootSectorSig0 != 0x55 || BR->bootSectorSig1 != 0xAA) {
                        //printf_P(PSTR("Bad sig? %02x %02x\r\n"), BR->bootSectorSig0, BR->bootSectorSig1);
                        st = -1;
                } else {
                        Offset = first;
                        storage = sto;
                        ffs = new FATFS;
                        ffs->pfat = this;
                        volmap = lv;
                        st = 0xff & f_mount(volmap, ffs);
                        if (!st) {
                                if (label != NULL) {
                                        delete label;
                                        label = NULL;
                                }
                                TCHAR path[4];
                                path[0] = '0' + lv;
                                path[1] = ':';
                                path[2] = '/';
                                path[3] = 0x00;
                                DWORD sn;
                                FRESULT t = f_getlabel(path, lb, &sn);
                                label = (uint8_t *)(operator new[] (13));
                                        label[0] = '/';
                                        label[1] = 0x00;
                                if (!t) {
                                        for (i = 0; lb[i] != 0x00 && i < 12; i++)
                                                label[i + 1] = lb[i];
                                        label[i + 1] = 0x00;
                                        // We will need to convert 'wide' chars, etc? yuck!
                                        // Life would be a whole lot easier if everything was just UTF-8!
                                }
                                //printf_P(PSTR("VOLUME %i @ '%s'\r\n"), volmap, &label[0]);
                        } else {
                                f_mount(volmap, NULL);
                                //printf_P(PSTR("Mount failed %i(%x)\r\n"), st, st);
                        }
                }
        }
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
DSTATUS PFAT::disk_initialize(void) {
        return disk_status();
}

DSTATUS PFAT::disk_status(void) {
        bool rc = storage->Status(storage);
        if (rc) return RES_OK;
        return STA_PROTECT;
}

DRESULT PFAT::disk_read(BYTE *buff, DWORD sector, BYTE count) {
        int rc = storage->Reads(sector, (uint8_t*)buff, storage, count);
        if (rc == 0) return RES_OK;
        return RES_ERROR;
}

DRESULT PFAT::disk_write(const BYTE *buff, DWORD sector, BYTE count) {
        int rc = storage->Writes(sector, (uint8_t*)buff, storage, count);
        if (rc == 0) return RES_OK;
        return RES_ERROR;
}

DRESULT PFAT::disk_ioctl(BYTE cmd, void* buff) {
        switch (cmd) {
                case CTRL_SYNC:
                        break;
                case GET_SECTOR_COUNT:
                        *(DWORD*)buff = storage->TotalSectors;
                        break;
                case GET_SECTOR_SIZE:
                        *(WORD*)buff = storage->SectorSize;
                        break;
                        //case CTRL_ERASE_SECTOR:
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

PFAT::~PFAT() {
        if (ffs != NULL) {
                f_mount(volmap, NULL);
                delete ffs;
                ffs = NULL;
        }

        if (label != NULL) {
                delete label;
                label = NULL;
        }
}

// Allow callbacks from C to C++ class methods.
extern "C" {

        DSTATUS CPP_PFAT_disk_initialize(PFAT *pfat) {
                return pfat->disk_initialize();
        }

        DSTATUS CPP_PFAT_disk_status(PFAT *pfat) {
                return pfat->disk_status();
        }

        DRESULT CPP_PFAT_disk_read(PFAT *pfat, BYTE *buff, DWORD sector, BYTE count) {
                return pfat->disk_read(buff, sector, count);
        }

        DRESULT CPP_PFAT_disk_write(PFAT *pfat, const BYTE *buff, DWORD sector, BYTE count) {
                return pfat->disk_write(buff, sector, count);
        }

        DRESULT CPP_PFAT_disk_ioctl(PFAT *pfat, BYTE cmd, void *buff) {
                return pfat->disk_ioctl(cmd, buff);
        }

        uint32_t CPP_PFAT_get_fattime(PFAT *pfat) {
                return pfat->get_fattime();
        }
}
