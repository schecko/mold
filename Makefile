src = $(wildcard *.c)
obj = $(src:.c=.o)

mold: $(obj)
	$(CC) -L/usr/local/lib -o $@ $^ -lgsl -lgslcblas

default: all
all: mold
	./mold

.PHONY: clean
clean:
	rm -f $(obj) mold
