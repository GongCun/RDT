DIRS = lib .

all:
	for i in $(DIRS); do \
		(cd $$i && echo "making $$i" && $(MAKE)) || exit 1; \
	done

clean:
	for i in $(DIRS) bin test; do \
		(cd $$i && echo "cleaning $$i" && $(MAKE) clean) || exit 1; \
	done
