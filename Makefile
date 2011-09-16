.PHONY: clean all cksum initcksum

all:
	$(MAKE) -C kernel

clean:
	$(MAKE) clean -C kernel
	$(MAKE) clean -C user
	$(MAKE) clean -C prog
