BUILD_TEST_DIR=build
BUILD_REL_DIR=build
BUILD_TEST_ASAN_DIR=build-asan

CONCURRENCY:=$(shell nproc)

FMT_BIN:=clang-format
CMAKE_BIN:=cmake

.PHONY: clean apply_fmt

clean:
	rm -fr $(BUILD_TEST_DIR)
	rm -fr $(BUILD_REL_DIR)
	rm -fr $(BUILD_TEST_ASAN_DIR)
	rm -fr _packages
	rm -fr _packages_bionic
	rm -fr _packages_jammy

apply_fmt:
	find ./ -maxdepth 5 -path './include/**.h' -o -path './src/**.c' -o -path './tests/**.c' | xargs -n 1 -t -P $(CONCURRENCY) $(FMT_BIN) -i

build_dbg: clean
	mkdir -p $(BUILD_TEST_DIR)
	cd $(BUILD_TEST_DIR) && $(CMAKE_BIN) -DCMAKE_BUILD_TYPE=Debug .. && make -j$(CONCURRENCY)

build_release: clean
	mkdir -p $(BUILD_REL_DIR)
	cd $(BUILD_REL_DIR) && $(CMAKE_BIN) -DCMAKE_BUILD_TYPE=Release .. && make -j$(CONCURRENCY)

build_asan: clean
	mkdir -p $(BUILD_TEST_ASAN_DIR)
	cd $(BUILD_TEST_ASAN_DIR) && $(CMAKE_BIN) -DCMAKE_BUILD_TYPE=ASAN .. && make -j$(CONCURRENCY)

build_release_deb: build_release
	cd $(BUILD_REL_DIR) && cpack -G DEB

build_dpkg_ubuntu_bionic:
	rm -fr _packages_bionic
	./docker/dpkg/docker-dpkg-ubuntu.sh bionic _packages_bionic

build_dpkg_ubuntu_jammy:
	rm -fr _packages_jammy
	./docker/dpkg/docker-dpkg-ubuntu.sh jammy _packages_jammy
