LDFLAGS = -framework Carbon -framework OpenGL -framework GLUT

julia : ../julia.cpp
	gcc -o $@ $< $(LDFLAGS)

clean:
	rm ./julia
