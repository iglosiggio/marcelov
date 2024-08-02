all: monaco.inc cream12.inc

clean:
	rm -f monaco.inc cream12.inc

.data.inc:
	../utils/to_data_file $< $@

.SUFFIXES: .data .inc

.PHONY: all clean
