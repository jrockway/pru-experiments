all: pru host

pru:
	make -C pru all

host:
	make -C host all

clean:
	make -C pru clean
	make -C host clean

.PHONY: all pru host clean
