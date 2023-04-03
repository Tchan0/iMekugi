################################################################################
# FOLDERS
BIN_FOLDER      = ../binaries

uname_m := $(shell uname -m) 

################################################################################
# COMPILERS
X86_COMPILER     = x86_64-linux-gnu-gcc
ARM_COMPILER     = aarch64-linux-gnu-gcc
ARM_FLAGS        = -march=armv8.2-a -mtune=cortex-a55 -DCACHELINE_SIZE=64 #Pine64's Quartz64 = ARM Cortex-A55
RISCV_COMPILER   = riscv64-linux-gnu-gcc
RISCV_FLAGS      = -march=rv64imafdc -mtune=sifive-7-series #Pine64's Star64 = StarFive JH7110 Quad-core 64-bit RISC-V SiFive U74 (RV64GC) Note: rv64imafdc=rv64gc 
WIN32_COMPILER   = i686-w64-mingw32-gcc
WIN64_COMPILER   = x86_64-w64-mingw32-gcc

################################################################################
# COMPILING
all: scsiserv scsiserv32exe scsiserv64exe dll
#    scsiservarm scsiservriscv

scsiserv:
	$(info Default machine is $(uname_m))
	$(X86_COMPILER) -o $(BIN_FOLDER)/scsiserv -I ../src scsiserv.c -lpthread

scsiserv32exe:
	$(WIN32_COMPILER) -o $(BIN_FOLDER)/scsiserv32.exe -I ../src -s scsiserv.c -lws2_32 -static-libgcc

scsiserv64exe:
	$(WIN64_COMPILER) -o $(BIN_FOLDER)/scsiserv64.exe -m64 -I ../src -s scsiserv.c -lws2_32 -static-libgcc

scsiservarm:
	$(ARM_COMPILER) -o $(BIN_FOLDER)/scsiservarm -I ../src scsiserv.c $(ARM_FLAGS) -lpthread

scsiservriscv:
	$(RISCV_COMPILER) -o $(BIN_FOLDER)/scsiservriscv -I ../src scsiserv.c $(RISCV_FLAGS) -lpthread

dll:
	$(WIN32_COMPILER) -o $(BIN_FOLDER)/wnaspi32.dll -I ../src -s -shared wnaspi32.c wnaspi32.def -Wl,--subsystem,windows -lws2_32 -static-libgcc

################################################################################
################################################################################
clean:
	rm -f $(BIN_FOLDER)/wnaspi32.dll
	rm -f $(BIN_FOLDER)/scsiserv $(BIN_FOLDER)/scsiserv32.exe $(BIN_FOLDER)/scsiserv64.exe $(BIN_FOLDER)/scsiservarm $(BIN_FOLDER)/scsiservriscv

################################################################################
# SENDING TO LOCAL DEVICE
# note: adapt to your ssh key, target ip/user/folder of your target device where the SCSI server needs to run
sendx86:
	scp -i ~/.ssh/id_rsa_scylla $(BIN_FOLDER)/scsiserv banjoo@192.168.1.170:~/iMekugi/
