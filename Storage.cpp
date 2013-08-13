#include <Usb.h>
#include <masstorage.h>
#include <Storage.h>

BulkOnly *Bulk[MAX_USB_MS_DRIVERS] = {
        &(BulkOnly(&Usb))
#if MAX_USB_MS_DRIVERS > 1
        , &(BulkOnly(&Usb))
#endif
#if MAX_USB_MS_DRIVERS > 2
        , &(BulkOnly(&Usb))
#endif
#if MAX_USB_MS_DRIVERS > 3
        , &(BulkOnly(&Usb))
#endif
#if MAX_USB_MS_DRIVERS > 4
        , &(BulkOnly(&Usb))
#endif
};

bool PStatus(storage_t *sto) {
        return (Bulk[((pvt_t *)sto->private_data)->B]->WriteProtected(((pvt_t *)sto->private_data)->lun));
}

int PRead(uint32_t LBA, uint8_t *buf, storage_t *sto) {
        uint8_t x = 0;
        int tries = FAT_MAX_ERROR_RETRIES;
        while (tries) {
                tries--;
                x = (Bulk[((pvt_t *)sto->private_data)->B]->Read(((pvt_t *)sto->private_data)->lun, LBA, (sto->SectorSize), 1, buf));
                if (!x) break;
        }
        int y = x;
        return y;
}

int PWrite(uint32_t LBA, uint8_t *buf, storage_t *sto) {
        int x = 0;
        int tries = FAT_MAX_ERROR_RETRIES;
        while (tries) {
                tries--;
                x = (Bulk[((pvt_t *)sto->private_data)->B]->Write(((pvt_t *)sto->private_data)->lun, LBA, sto->SectorSize, 1, buf));
                if (x == MASS_ERR_WRITE_PROTECTED) break;
                if (!x) break;
        }
        int y = x;
        return y;
}

int PReads(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        uint8_t x = 0;
        int tries = FAT_MAX_ERROR_RETRIES;
        while (tries) {
                tries--;
                x = (Bulk[((pvt_t *)sto->private_data)->B]->Read(((pvt_t *)sto->private_data)->lun, LBA, (sto->SectorSize), count, buf));
                if (!x) break;
                delay(200);
        }
        int y = x;
        return y;
}

int PWrites(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        int x = 0;
        int tries = FAT_MAX_ERROR_RETRIES;
        while (tries) {
                tries--;
                x = (Bulk[((pvt_t *)sto->private_data)->B]->Write(((pvt_t *)sto->private_data)->lun, LBA, sto->SectorSize, count, buf));
                if (x == MASS_ERR_WRITE_PROTECTED) break;
                if (!x) break;
                delay(200);
        }
        int y = x;
        return y;
}

