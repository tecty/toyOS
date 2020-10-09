#
# Copyright (C) 2018 bzt (bztsrc@github)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
#

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -Wextra -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
CROSS = aarch64-linux-gnu-

all: run 

start.o: start.S
	$(CROSS)as -c start.S -o start.o

%.o: %.c
	$(CROSS)gcc $(CFLAGS) -c $< -o $@

kernel.elf: start.o $(OBJS) link.ld
	$(CROSS)ld -nostdlib -nostartfiles start.o $(OBJS) -T link.ld -o $@
	
kernel.img: kernel.elf
	$(CROSS)objcopy -O binary $< $@

clean:
	rm $(OBJS) 

run: kernel.img
	qemu-system-aarch64 -M raspi3 -kernel kernel.img -serial stdio 