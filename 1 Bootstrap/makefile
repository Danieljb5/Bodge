main: obj/main.o obj/lexer.o obj/parser.o
	g++ -o $@ $^

obj/%.o: src/%.cpp
	@mkdir -p obj
	g++ -o $@ -c $^ -Isrc

.PHONY: clean

clean:
	rm obj main -r || true