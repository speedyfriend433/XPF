CC = clang

CHOMA_DYLIB_PATH ?= external/ios/lib
CFLAGS = -O2
CFLAGS_MACOS = -Iexternal/include -Isrc/tspf -Iexternal/ios/include
CFLAGS_IOS = -Iexternal/ios/include -Isrc/tspf -isysroot $(shell xcrun --sdk iphoneos --show-sdk-path) -miphoneos-version-min=15.0 -arch arm64 -arch arm64e
LDFLAGS = -lcompression

TSPF_SRCS = $(wildcard src/tspf/*.c src/tspf/plugins/*.c)
TSPF_OBJS_IOS = $(TSPF_SRCS:.c=.ios.o)
TSPF_OBJS_MACOS = $(TSPF_SRCS:.c=.macos.o)

all: libxpf_macos.dylib libxpf.dylib xpf_test_macos xpf_test_ios patch_app

clean: 
	@rm -f libxpf.dylib
	@rm -f libxpf_macos.dylib
	@rm -f xpf_test_ios
	@rm -f xpf_test_macos
	@rm -f patch_app
	@rm -f $(TSPF_OBJS_IOS)
	@rm -f $(TSPF_OBJS_MACOS)

libxpf_macos.dylib: $(wildcard src/*.c external/lib/libchoma.a) $(TSPF_OBJS_MACOS)
	$(CC) $(CFLAGS) $(CFLAGS_MACOS) $(LDFLAGS) -dynamiclib -install_name @loader_path/libxpf_macos.dylib -o $@ $^

libxpf.dylib: $(wildcard src/*.c) $(TSPF_OBJS_IOS)
	$(CC) $(CFLAGS) $(CFLAGS_IOS) $(LDFLAGS) -dynamiclib -L$(CHOMA_DYLIB_PATH) -lchoma -install_name @loader_path/libxpf.dylib -o $@ $^
	ldid -S $@

%.ios.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_IOS) -c -o $@ $<

%.macos.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_MACOS) -c -o $@ $<

xpf_test_macos: $(wildcard src/cli/*.c external/lib/libchoma.a)
	$(CC) $(CFLAGS) $(CFLAGS_MACOS) $(LDFLAGS) -L. -lxpf_macos -o $@ $^

xpf_test_ios: $(wildcard src/cli/*.c) 
	$(CC) $(CFLAGS) $(CFLAGS_IOS) $(LDFLAGS) -L. -lxpf -o $@ $^

patch_app: src/tspf/test/patch_app.c $(TSPF_OBJS_IOS)
	$(CC) $(CFLAGS) $(CFLAGS_IOS) $(LDFLAGS) -L. -L$(CHOMA_DYLIB_PATH) -lxpf -lchoma -o $@ $^
	ldid -S $@

all: libxpf_macos.dylib libxpf.dylib xpf_test_macos xpf_test_ios patch_app

clean: 
	@rm -f libxpf.dylib
	@rm -f libxpf_macos.dylib
	@rm -f xpf_test_ios
	@rm -f xpf_test_macos
	@rm -f patch_app
	@rm -f $(TSPF_OBJS_IOS)
	@rm -f $(TSPF_OBJS_MACOS)