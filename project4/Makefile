cs311cache: main.c
	gcc -g -O2 $^ -o $@
clean:
	rm -rf cs311cache*

test: cs311cache test_lru test_lflru

test_lru: cs311cache test_lru_simple test_lru_milc test_lru_gcc test_lru_libquantum

test_lflru: cs311cache test_lflru_simple test_lflru_milc test_lflru_gcc test_lflru_libquantum

test_lru_simple:
	@echo "Testing LRU/simple"; \
        ./cs311cache -c 1024:8:8 -x -r sample_input/simple | diff -Naur sample_output/LRU/simple - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_lru_milc:
	@echo "Testing LRU/milc"; \
        ./cs311cache -c 1024:8:8 -x -r sample_input/milc | diff -Naur sample_output/LRU/milc - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
	

test_lru_gcc:
	@echo "Testing LRU/gcc"; \
        ./cs311cache -c 1024:8:8 -x -r sample_input/gcc | diff -Naur sample_output/LRU/gcc - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
	

test_lru_libquantum:
	@echo "Testing LRU/libquantum"; \
        ./cs311cache -c 1024:8:8 -x -r sample_input/libquantum | diff -Naur sample_output/LRU/libquantum - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_lflru_simple:
	@echo "Testing LFLRU/simple"; \
        ./cs311cache -c 1024:8:8 -x sample_input/simple | diff -Naur sample_output/LFLRU/simple - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_lflru_milc:
	@echo "Testing LFLRU/milc"; \
        ./cs311cache -c 1024:8:8 -x sample_input/milc | diff -Naur sample_output/LFLRU/milc - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
	
test_lflru_gcc:
	@echo "Testing LFLRU/gcc"; \
        ./cs311cache -c 1024:8:8 -x sample_input/gcc | diff -Naur sample_output/LFLRU/gcc - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
	

test_lflru_libquantum:
	@echo "Testing LFLRU/libquantum"; \
        ./cs311cache -c 1024:8:8 -x sample_input/libquantum | diff -Naur sample_output/LFLRU/libquantum - ;\
        if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi	
