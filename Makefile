src = $(wildcard *.c)
obj = $(src:.c=.o)

mold: $(obj)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) myprog
