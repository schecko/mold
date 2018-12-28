src = $(wildcard *.c)
obj = $(src:.c=.o)

mold: $(obj)
	$(CC) -o $@ $^

default: all
all: mold
	./mold

.PHONY: clean
clean:
	rm -f $(obj) mold
