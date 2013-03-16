generic_storage
===============

Generic partition and filesystem hooks for Arduino

Currently supports FAT 16/32, and long filenames.

Built-in support for USB shield from http://www.circuitsathome.com
See https://github.com/felis/USB_Host_Shield_2.0 for drivers.

NOTE: The USB host driver is not 100% complete or compatable with all devices.
Some are known to misbehave. Efforts to resolve the problems are in progress.
Some USB thumb drives will work, as will most card readers. 
It depends on the SCSI command set that is supported by the device.

