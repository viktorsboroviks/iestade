.PHONY: \
	all \
	examples \
	format \
	format-cpp \
	format-json \
	lint \
	lint-cpp \
	clean

all: examples

examples: config.o csv.o

config.o: \
		rododendrs \
		examples/config.cpp
	g++ -Wall -Wextra -Werror -Wpedantic \
		-std=c++20 -O3 \
		-I./include \
		-I./rododendrs/include \
		examples/config.cpp -o $@

csv.o: \
		rododendrs \
		examples/csv.cpp \
		examples/data_struct.csv \
		examples/data_vector.csv
	g++ -Wall -Wextra -Werror -Wpedantic \
		-std=c++20 -O3 \
		-I./include \
		-I./rododendrs/include \
		examples/csv.cpp -o $@

rododendrs:
	git clone git@github.com:viktorsboroviks/rododendrs.git
	cd rododendrs; git checkout v1.32

format: format-cpp format-json

format-cpp: \
		include/iestaade.hpp \
		examples/config.cpp \
		examples/csv.cpp
	clang-format -i $^

format-json: examples/config.json
	jq . examples/config.json | sponge examples/config.json

lint: lint-cpp

lint-cpp: \
		include/iestaade.hpp \
		examples/config.cpp \
		examples/csv.cpp
	cppcheck \
		--enable=warning,portability,performance \
		--enable=style,information \
		--inconclusive \
		--library=std,posix,gnu \
		--platform=unix64 \
		--language=c++ \
		--std=c++20 \
		--inline-suppr \
		--check-level=exhaustive \
		--suppress=checkersReport \
		--checkers-report=cppcheck_report.txt \
		-I./include \
		$^

clean:
	rm -rf `find . -name "*.o"`

distclean: clean
	rm -rf rododendrs
