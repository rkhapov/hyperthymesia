BUILD_TEST_DIR=build
BUILD_REL_DIR=build
HM_DIR=$(PWD)
COMPILE_CONCURRENCY=16

FMT_BIN:=clang-format
CMAKE_BIN:=cmake

.PHONY: clean apply_fmt

clean:
	rm -fr $(BUILD_TEST_DIR)
	rm -fr $(BUILD_REL_DIR)

apply_fmt:
	find ./ -maxdepth 5 -iname '*.h' -o -iname '*.c' | xargs -n 1 -t -P $(COMPILE_CONCURRENCY) $(FMT_BIN) -i

build_tests_dbg: clean
	mkdir -p $(BUILD_TEST_DIR)
	cd $(BUILD_TEST_DIR) && $(CMAKE_BIN) -DHT_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug $(HM_DIR) && make -j$(COMPILE_CONCURRENCY)

build_tests_release: clean
	mkdir -p $(BUILD_REL_DIR)
	cd $(BUILD_REL_DIR) && $(CMAKE_BIN) -DHT_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release $(HM_DIR) && make -j$(COMPILE_CONCURRENCY)
