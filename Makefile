BUILD_TEST_DIR=build
BUILD_REL_DIR=build
HM_DIR=$(PWD)

FMT_BIN:=clang-format
CMAKE_BIN:=cmake

.PHONY: clean apply_fmt

clean:
	rm -fr $(BUILD_TEST_DIR)
	rm -fr $(BUILD_REL_DIR)

apply_fmt:
	find ./ -maxdepth 5 -iname '*.h' -o -iname '*.c' | xargs $(FMT_BIN) -i
