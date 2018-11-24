CC=emcc
SRCFOLDER=/mnt/d/GitHub/video_encoder_for_ogl_dx/SDL_App
APPFOLDER=$(SRCFOLDER)/SDL_App
LIBFOLDER=$(SRCFOLDER)/SDLLib
SOURCES:=$(wildcard $(SRCFOLDER)/ext/src/*.cpp)
SOURCES+=$(wildcard $(SRCFOLDER)/src/*.c)
SOURCES+=$(wildcard $(SRCFOLDER)/SDLLib/*.cpp)
SOURCES+=$(wildcard *.cpp)
LDFLAGS=-O2 --llvm-opts 2
OUTPUT=../WebApp/wwwroot/SDL_App.html
EMCC_DEBUG=0

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(SOURCES) 
	$(CC) $(SOURCES) --bind -s WASM=0 -s USE_ZLIB=1 -s FULL_ES2=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=1 --memory-init-file 0 -D__EMSCRIPTEN__ -std=c++11 -I $(SRCFOLDER)/ext/glm-0.9.8.4/ -I $(SRCFOLDER)/ext/include/ -I $(LIBFOLDER)  $(LDFLAGS) -o $(OUTPUT)

clean:
	rm $(OUTPUT)
