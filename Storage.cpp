#include <Usb.h>
#include <masstorage.h>
#include <Storage.h>

// TO-DO: Cache all requests into a single pool
// Problem: Caches must handle all types, not just USB
// Perhaps some sort of tag system is needed.


#ifdef _usb_h_
BulkOnly *UHS_USB_BulkOnly[MAX_USB_MS_DRIVERS];

/**
 * This must be called before using UHS USB Mass Storage. This works around a G++ bug.
 * Thanks to Lei Shi for the heads up.
 */
static void UHS_USB_BulkOnly_Init(void) {
        for(int i = 0; i < MAX_USB_MS_DRIVERS; i++) {
                UHS_USB_BulkOnly[i] = new BulkOnly(&Usb);
        }
}

// On mass storage, there is nothing to do. Just call the status function.

DSTATUS UHS_USB_BulkOnly_Initialize(storage_t *sto) {
        if((UHS_USB_BulkOnly[((pvt_t *)sto->private_data)->B]->WriteProtected(((pvt_t *)sto->private_data)->lun))) {
                return STA_PROTECT;
        } else {
                return STA_OK;
        }
}

bool UHS_USB_BulkOnly_Status(storage_t *sto) {
        return (UHS_USB_BulkOnly[((pvt_t *)sto->private_data)->B]->WriteProtected(((pvt_t *)sto->private_data)->lun));
}


// TO-DO: Split multiple to single r/w and cache for larger systems.

int UHS_USB_BulkOnly_Read(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        uint8_t x = 0;
        int tries = FAT_MAX_ERROR_RETRIES;
        while(tries) {
                tries--;
                x = (UHS_USB_BulkOnly[((pvt_t *)sto->private_data)->B]->Read(((pvt_t *)sto->private_data)->lun, LBA, (sto->SectorSize), count, buf));
                if(!x) break;
                delay(200);
        }
        int y = x;
        return y;
}

int UHS_USB_BulkOnly_Write(uint32_t LBA, uint8_t *buf, storage_t *sto, uint8_t count) {
        int x = 0;
        int tries = FAT_MAX_ERROR_RETRIES;
        while(tries) {
                tries--;
                x = (UHS_USB_BulkOnly[((pvt_t *)sto->private_data)->B]->Write(((pvt_t *)sto->private_data)->lun, LBA, sto->SectorSize, count, buf));
                if(x == MASS_ERR_WRITE_PROTECTED) break;
                if(!x) break;
                delay(200);
        }
        int y = x;
        return y;
}

#endif


// YOUR DRIVERS HERE








// Allow init to happen only once in the case of multiple external inits.
static bool inited = false;

/**
 * This must be called before using generic_storage. Calling more than once is harmless.
 */
void Init_Generic_Storage(void) {
        if(!inited) {
                inited = true;
#ifdef _usb_h_
                UHS_USB_BulkOnly_Init();
#endif

// YOUR INIT HERE

        }
        return;
}

#if 0

void PollStorage(void) {
#ifdef _usb_h_
        UHS_USB_BulkOnly_Poll();
#endif
// YOUR POLLING HERE

}
#endif
