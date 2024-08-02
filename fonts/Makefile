all: monaco.inc

clean:
	rm -f monaco.inc

monaco.inc: monaco.font
	../utils/to_data_file $< $@

.PHONY: all clean
