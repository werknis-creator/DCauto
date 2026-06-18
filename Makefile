TARGET = my_game.elf
OBJS = main.o

all: rm-elf $(TARGET)

include $(KOS_BASE)/Makefile.rules

rm-elf:
	rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(KOS_CC) $(KOS_CFLAGS) $(KOS_LDFLAGS) -o $@ $(KOS_START) $^ $(KOS_LIBS) -LGLdc -lGLdc -lm

clean:
	rm -f $(TARGET) $(OBJS) romdisk.*
