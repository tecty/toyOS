NAME		= first-steps.elf
CROSS		= aarch64-linux-gnu-
CFLAGS		= -ggdb3 -std=c17 -Wall -nostdlib -O0
LDFLAGS 	= -Bstatic --gc-sections -nostartfiles -nostdlib
QEMU		= qemu-system-aarch64
QEMU_FLAGS	= -machine virt -m 1024M -cpu cortex-a53 -nographic -s
OBJS = main.o startup.o trap.o ctrap.o io.o

all: $(NAME)


%.o: %.asm
	${CROSS}as -o $@ $^

%.o: %.c
	${CROSS}gcc ${CFLAGS} -c -o $@ $^

$(NAME): $(OBJS)
	${CROSS}ld $(LDFLAGS) -o $@ -T link.ld $^


clean: 
	rm -f $(NAME) *.o

qemu: $(NAME)
	$(QEMU) $(QEMU_FLAGS) -kernel $(NAME)

qemu-stop: $(NAME)
	$(QEMU) $(QEMU_FLAGS) -S -kernel $(NAME)
