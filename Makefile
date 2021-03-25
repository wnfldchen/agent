# Copyright 2019, Winfield Chen and Lloyd T. Elliott.

INCLUDE := ./include ./lib/include /usr/local/include ./lib/zstd-1.4.4 ./lib/zstd-1.4.4/common $(HOME)/opt/include/
LIBDIRS := /usr/local/lib
LIBRARY += ./lib/zstd-1.4.4 ${MKLROOT}/lib/intel64
LINK := gsl gslcblas m zstd mkl_intel_ilp64 mkl_sequential mkl_core dl 
COMMON := -DNDEBUG -DMKL_ILP64 -march=native -m64 -flto -fuse-linker-plugin -O3 -march=native -fno-trapping-math -funsafe-math-optimizations -fno-rounding-math -fcx-limited-range -fno-signed-zeros -floop-nest-optimize -fno-signaling-nans -static-libstdc++ -D_FORTIFY_SOURCE=2 -fomit-frame-pointer -static-libgcc -static -I${MKLROOT}/include -L${MKLROOT}/lib/intel64 -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core -lpthread -Wl,--no-as-needed -lm -ldl
COMMON_FLAGS += -DNDEBUG -DMKL_ILP64 -pthread -march=native -m64 -flto -fuse-linker-plugin -O3 -march=native -fno-trapping-math -funsafe-math-optimizations -fno-rounding-math -fcx-limited-range -fno-signed-zeros -floop-nest-optimize -fno-signaling-nans -static-libstdc++ -D_FORTIFY_SOURCE=2 -fomit-frame-pointer -static-libgcc -static -I${MKLROOT}/include -L${MKLROOT}/lib/intel64 -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core -lpthread -Wl,--no-as-needed -lm -ldl
CC := gcc
CPPFLAGS += -MD -MP
CFLAGS += -DBUILD_DATE='"$(shell date "+%B %Y")"' -DGIT_VERSION='"$(shell git describe --always --abbrev=10)"' $(COMMON) $(foreach i,$(INCLUDE),-I$(i))
SRCDIRS := ./src
SOURCES := $(wildcard $(foreach i,$(SRCDIRS),$i/*.c))

export ZSTD_LEGACY_SUPPORT := 0
export ZSTD_LIB_DICTBUILDER := 0
export ZSTD_LIB_DEPRECATED := 0

agent: $(SOURCES:%.c=%.o) | zstd
	$(CC) -o $@ $(COMMON_FLAGS) $(foreach i,$(LIBRARY),-L$i) $^ $(foreach i,$(LINK),-l$(i))

.PHONY: zstd
zstd:
	$(MAKE) -C lib/zstd-1.4.4 libzstd.a

.PHONY: clean
clean:
	$(MAKE) -C lib/zstd-1.4.4 clean
	rm $(foreach i,$(SRCDIRS),$i/*.o) $(foreach i,$(SRCDIRS),$i/*.d) || true

-include $(SOURCES:%.c=%.d)
