KERNEL = $(shell uname -r)
obj-m += char_dev.o

build: kernel_modules
kernel_modules:
	make -C /lib/modules/$(KERNEL)/build M=$(CURDIR) modules
clean:
	make -C /lib/modules/$(KERNEL)/build M=$(CURDIR) clean