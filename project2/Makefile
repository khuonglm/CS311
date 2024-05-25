cs311sim: cs311.c util.c parse.c run.c
	gcc -g -O2 $^ -o $@

.PHONY: clean test help
clean:
	rm -rf *~ cs311sim

help:
	@echo "The following options are provided with Make\n\t-make:\t\tbuild simulator\n\t-make clean:\tclean the build\n\t-make test:\ttest your simulator"

test: cs311sim test_1 test_2 test_3 test_4 test_5

test_1:
	@echo "Testing example1"; \
	./cs311sim -m 0x10000000:0x10000010 -n 50 sample_input/example1-leaf.o | diff -Naur sample_output/example1-leaf - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_2:
	@echo "Testing example2"; \
	./cs311sim -m 0x10000000:0x10000010 -n 50 sample_input/example2-j.o | diff -Naur sample_output/example2-j - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_3:
	@echo "Testing example3"; \
	./cs311sim -m 0x00400000:0x00400010 -n 100 sample_input/example3-fact.o | diff -Naur sample_output/example3-fact - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_4:
	@echo "Testing example4"; \
	./cs311sim -m 0x10000000:0x10000010 -n 100 sample_input/example4-mem.o | diff -Naur sample_output/example4-mem - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_5:
	@echo "Testing example5"; \
	./cs311sim -m 0x10000000:0x10000010 -n 100 sample_input/example5-slt.o | diff -Naur sample_output/example5-slt - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
