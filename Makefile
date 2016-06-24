#odv : odv.c get_conf.c ip.c
#	gcc -o odv -pthread odv.c get_conf.c ip.c -lpthread -Wall
#reset : reset.c get_conf.c
#	gcc -o reset reset.c get_conf.c -Wall
all: odv reset
odv: odv.c get_conf.c ip.c
	gcc -o odv -pthread odv.c get_conf.c ip.c -lpthread -Wall
reset: reset.c get_conf.c
	gcc -o reset reset.c get_conf.c
clean:
	rm -fr odv reset
