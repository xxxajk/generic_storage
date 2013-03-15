/*
 * File:   Storage.h
 * Author: root
 *
 * Created on February 19, 2013, 7:44 AM
 */

#ifndef STORAGE_H
#define	STORAGE_H

#define MAX_HUBS 4 // must be at least 1
#define MAX_PARTS (4*MAX_HUBS) // be sure to also adjust any file system limits

/*
 * Notes:
 * Read and Write do not care about sector counts, or how many to read.
 * The passed method needs to just read and write one full sector.
 * Reads and Writes are for multiple sectors.
 *
 * In order to assist these calls, a pointer to the Storage struct is also passed,
 * so that your driver can get at its own private information, if used.
 * The private_data pointer can point to anything that you need for your driver,
 * or nothing at all if you do not need any. Nothing in the code here will use it.
 *
 * Also, "sector" is the Logical Block Address on the media, starting at ZERO.
 * C/H/S is not, and will not be used!
 * Any translations from the LBA must be done on the storage driver side, if needed.
 * Translations include any sector offsets, logical device numbers, etc.
 * The buffer size is guaranteed to be correct providing that you set SectorSize properly.
 *
 * Success from Read and Write should return ZERO,
 * Errors should return a non-Zero integer meaningful to the storage caller.
 * Negative One is returned by this layer to indicate some other error.
 *
 *
 */


struct Storage {
        int (*Read)(uint32_t, uint8_t *, struct Storage *); // single sector read
        int (*Write)(uint32_t, uint8_t *, struct Storage *); // single sector write
        int (*Reads)(uint32_t, uint8_t *, struct Storage *, uint8_t); // multiple sector read
        int (*Writes)(uint32_t, uint8_t *, struct Storage *, uint8_t); // multiple sector write
        uint16_t SectorSize; // physical or translated size on the physical media
        uint32_t TotalSectors; // Total sector count. Used to guard against illegal access.
        void *private_data; // Anything you need, or nothing at all.
};

typedef Storage storage_t;
#ifdef _usb_h_

BulkOnly *Bulk[MAX_HUBS];

typedef struct Pvt {
        uint8_t lun;
        int B; // which "BulkOnly" instance
        // these are for 'superblock' access.
        uint8_t *label; // Volume label NULL for /
        uint8_t volmap; // FatFS volume number
} pvt_t;

pvt_t info[MAX_PARTS];

int PRead(uint32_t LBA, uint8_t *buf, storage_t *sto) {
        uint8_t x = 0;
        int tries = 3;
        //uint8_t sb[64];
        while(tries) {
                tries--;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Read(((pvt_t *) sto->private_data)->lun, LBA, (sto->SectorSize), 1, buf));
                //uint8_t v = (Bulk[((pvt_t *) sto->private_data)->B]->RequestSense(((pvt_t *) sto->private_data)->lun,63,sb));
                if(UsbDEBUGlvl >= 0x64) {
                        if(!x) {
                                if(UsbDEBUGlvl > 0x68) {
                                        for(int i = 0; i < sto->SectorSize; i++) {
                                                printf("%02x  ", buf[i]);
                                        }
                                        printf("\r\n");
                                }
                        } else {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
                if(!x) break;
        }

        if(!x && UsbDEBUGlvl >= 0x64) printf("READ Success after %u tries\r\n", 3 - tries);
        int y = x;
        return y;
}

int PWrite(uint32_t LBA, uint8_t *buf, storage_t *sto) {
        //uint8_t sb[64];
        int x = 0;
        int tries = 3;
        while(tries) {
                tries--;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Write(((pvt_t *) sto->private_data)->lun, LBA, sto->SectorSize, 1, buf));
                //uint8_t v = (Bulk[((pvt_t *) sto->private_data)->B]->RequestSense(((pvt_t *) sto->private_data)->lun,63,sb));
                if(UsbDEBUGlvl > 0x64) {
                        if(x) {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
                if(!x) break;
        }
        if(!x && UsbDEBUGlvl >= 0x64) printf("Write Success after %u tries\r\n", 3 - tries);
        int y = x;
        return y;
}

int PReads(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        //uint8_t sb[64];
        uint8_t x = 0;
        int tries = 3;
        while(tries) {
                tries--;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Read(((pvt_t *) sto->private_data)->lun, LBA, (sto->SectorSize), count, buf));
                //uint8_t v = (Bulk[((pvt_t *) sto->private_data)->B]->RequestSense(((pvt_t *) sto->private_data)->lun,63,sb));
                if(UsbDEBUGlvl >= 0x64) {
                        if(!x) {
                                if(UsbDEBUGlvl > 0x68) {
                                        for(int i = 0; i < sto->SectorSize; i++) {
                                                printf("%02x  ", buf[i]);
                                        }
                                        printf("\r\n");
                                }
                        } else {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
                if(!x) break;

        }

        if(!x && UsbDEBUGlvl >= 0x64) printf("Reads Success after %u tries\r\n", 3 - tries);
        int y = x;
        return y;
}

int PWrites(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        //uint8_t sb[64];
        int x = 0;
        int tries = 3;
        while(tries) {
                tries--;
                x = (Bulk[((pvt_t *) sto->private_data)->B]->Write(((pvt_t *) sto->private_data)->lun, LBA, sto->SectorSize, count, buf));
                //uint8_t v = (Bulk[((pvt_t *) sto->private_data)->B]->RequestSense(((pvt_t *) sto->private_data)->lun,63,sb));
                if(UsbDEBUGlvl > 0x64) {
                        if(x) {
                                printf("ERROR 0x%02x\r\n", x);
                        }
                }
                if(!x) break;
        }
        if(!x && UsbDEBUGlvl >= 0x64) printf("Writes Success after %u tries\r\n", 3 - tries);
        int y = x;
        return y;
}

#endif
// Your stuff here...

#endif	/* STORAGE_H */

