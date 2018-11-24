CC=emcc
SOURCES:=$(wildcard ~/app/SDL_App/ext/src/*.cpp)
SOURCES+=$(wildcard ~/app/SDL_App/ext/src/*.c)
SOURCES+=$(wildcard ~/app/SDL_App/SDLLib/*.cpp)
SOURCES+=$(wildcard ~/app/SDL_App/CommonSrcFiles/*.cpp)
SOURCES+=$(wildcard *.cpp)
LDFLAGS=-O2 --llvm-opts 2
OUTPUT=../WebApp/SDL_App.html
EMCC_DEBUG=1

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(SOURCES) 
	$(CC) $(SOURCES) --bind -s WASM=1 -s USE_ZLIB=1 -s FULL_ES2=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=1 --memory-init-file 0 -D__EMSCRIPTEN__ -std=c++11 -I ~/app/SDL_App/ext/glm-0.9.8.4/ -I ~/app/SDL_App/ext/include/ -I ~/app/SDL_App/SDLLib/ -I ~/app/SDL_App/CommonSrcFiles/ $(LDFLAGS) -o $(OUTPUT)

clean:
	rm $(OUTPUT)
