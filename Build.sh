#!/bin/bash
set -ex

# 1. Generowanie Makefile wewnatrz kontenera (Gwarantuje poprawne TABULATORY!)
cat << 'EOF' > Makefile
TARGET = my_game.elf
OBJS = main.o
all: rm-elf $(TARGET)
include $(KOS_BASE)/Makefile.rules
rm-elf:
	rm -f $(TARGET)
$(TARGET): $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ $(KOS_LIBS) -lm -lGL
clean:
	rm -f $(TARGET) $(OBJS) romdisk.*
EOF

# 2. Srodowisko KOS i Kompilacja
. /opt/toolchains/dc/kos/kos/environ.sh || true
make

# 3. Pakowanie
sh-elf-objcopy -R .stack -O binary my_game.elf main.bin
scramble main.bin 1ST_READ.BIN

echo "Hardware ID   : SEGA SEGAKATANA" > ip.txt
echo "Maker ID      : SEGA ENTERPRISES" >> ip.txt
echo "Device Info   : 0x0000 GD-ROM1/1" >> ip.txt
echo "Area Symbols  : 0x00000000" >> ip.txt
echo "Peripherals   : 0x00000000" >> ip.txt
echo "Product Number: 0000000000000000" >> ip.txt
echo "Version       : V1.000" >> ip.txt
echo "Release Date  : 20230101" >> ip.txt
echo "Boot Filename : 1ST_READ.BIN" >> ip.txt
echo "SW Maker Name : GITHUB" >> ip.txt
echo "Game Title    : GTA SA DC" >> ip.txt

makeip ip.txt IP.BIN
genisoimage -V MOJA_GRA -G IP.BIN -joliet -rock -l -o mygame.iso 1ST_READ.BIN
cdi4dc mygame.iso MojaGra_GTA.cdi -d
