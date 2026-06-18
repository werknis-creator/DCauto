TARGET = my_game.elf
OBJS = main.o

all: rm-elf $(TARGET)

# Dołączenie standardowych reguł KallistiOS
include $(KOS_BASE)/Makefile.rules

rm-elf:
	rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ $(KOS_LIBS)

clean:
	rm -f $(TARGET) $(OBJS) romdisk.*
