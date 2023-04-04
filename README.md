# iMekugi
replacement for wnaspi32.dll for modern systems, and remote (TCP) communication

## 1. What
* For regular users:
  * wnaspi32.dll replacement for modern systems
* For advanced users:
  * can expose devices as if they were linked to another host adapter
    * This is particularly useful for old SCSI programs that only support host adapter id 0
  * you can now launch your legacy SCSI program from a pc/laptop without SCSI card, and communicate with a SCSI device connected to another computer that has a SCSI card.
* For Virtual Machine users:
  * can forward SCSI calls made to wnaspi32.dll from a VM to the host pc or another pc, via TCP
    * the host or other pc can be running Windows or Linux
* For developers:
  * SCSI calls can be logged to a file, allowing to reverse-engineer custom protocols

![iMekugi Architecture](/docs/iMekugiAspi.jpg "iMekugi Architecture")

## 2. ! WARNINGS !
* Use at your own risk. No guarantees made.
* No security is provided for the packets going via TCP, so this is not secure whatsoever.
* If you expose your SCSI device to somebody else via TCP, be aware that this other person could ruin your hardware/software/data/anything in your life and more.

## 3. How To Use - As a wnaspi32.dll replacement
* Copy the iMekugi wnaspi32.dll to the folder of your SCSI program
* Run your SCSI program a first time:
  * your SCSI program will say something like "no device detected", that's normal.
  * wnaspi32.dll will create a config file (wnaspi32.cfg) & a log file (wnaspi32.log)
* Run your SCSI program a 2nd time, this time it should work
  * if you still have problems, check the wnaspi32.log file.
    * the detail level of the logging can be increased by changing the last 2 parameters in the wnaspi32.cfg file:
      * enable_logging: 1=core, 2=low, 3=full detail
      * logging_file_max_MB_size: maximum file size for the log file (default = 50 MB)

## 4. How To Use - To expose a device as if it were linked to another host adapter id
* Copy the iMekugi wnaspi32.dll to the folder of your SCSI program
* Run your SCSI program a first time:
  * your SCSI program will say something like "no device detected", that's normal.
  * wnaspi32.dll will create a config file (wnaspi32.cfg) & a log file (wnaspi32.log)
* Edit wnaspi32.cfg, and change the first 2 parameters:
    * tcp_enabled: set to 1
    * target_ip: set to 127.0.0.1
* Run the iMekugi SCSI server application (scsiserv32.exe, scsiserv64.exe or scsiserv) a first time - AS ADMINISTRATOR
  * This will create a configuration file scsiserv.cfg, and exit, this is normal.
    * This will be prefilled with devices found on the system. If you forgot to connect your SCSI device at this point, just delete scsiserv.cfg, and try again.
      * Under Windows, you need to connect the SCSI devices when the pc boots, so you'll have to reboot
      * Under Linux, a rescan of the SCSI bus can be launched via: sudo rescan-scsi-bus
        * Note that "lsscsi" is needed on your system (eg: sudo apt-get install -y lsscsi)
        * You might also want the "rescan-scsi-bus" script (eg: sudo apt-get install -y scsitools)
        * eg to launch it with detailed max 50MB log, listening on port 7032:  sudo ./scsiserv 7032 3 50
        * You might need to do a chmod to change the device access rights, eg:
          * lsscsi -g
          * ls -al /dev/sg*
          * sudo chmod 777 /dev/sg*
          * ls -al /dev/sg*
  * Edit scsiserv.cfg to map the detected SCSI devices to the desired host adapter id/target id/lun id
  * "AS ADMINISTRATOR" is needed, because we're doing low-level access to the SCSI device 
* Run the iMekugi SCSI server application (scsiserv32.exe, scsiserv64.exe or scsiserv) a 2nd time - AS ADMINISTRATOR
  * This time the iMekugi SCSI server application should be waiting for a connection
  * "AS ADMINISTRATOR" is needed, because we're doing low-level access to the SCSI device 
* Run your SCSI program a 2nd time, this time it should work
  * if you still have problems, check the wnaspi32.log file.
    * the detail level of the logging can be increased by changing the last 2 parameters in the wnaspi32.cfg file:
      * enable_logging: 1=core, 2=low, 3=full detail
      * logging_file_max_MB_size: maximum file size for the log file (default = 50 MB)

## 5. How To Use - From within a Virtual Machine
* Same as "4. How To Use - To expose a device as if it were linked to another host adapter id", but:
  * wnaspi32.cfg: set the target ip of the pc where the iMekugi server will be running
  * launch the iMekugi SCSI server application (scsiserv32.exe, scsiserv64.exe or scsiserv) on your host pc, or any pc that the Virtual Machine can connect to via TCP
    * don't forget to launch the iMekugi SCSI server application AS ADMINISTRATOR

## 6. How to build
* wnaspi32.dll, scsiserv32.exe & scsiserv65.exe:
  * you can find them pre-compiled in the zip file in the /binaries folder
* scsiserv (Linux version of the iMekugi SCSI server:
  * make clean
  * make
  * To send it to a target device: make sendx86 (adapt the makefile to your needs for this)
