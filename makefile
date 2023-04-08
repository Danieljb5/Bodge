FLAGS = -g
LINKS = -lstdc++fs

bodge: obj/main.o obj/lexer.o obj/parser.o obj/cbackend.o
	g++ -o $@ $^ $(FLAGS) $(LINKS)

obj/%.o: src/%.cpp
	@mkdir -p obj
	g++ -o $@ -c $^ -Isrc $(FLAGS)

.PHONY: clean

clean:
	rm obj bodge -r || true