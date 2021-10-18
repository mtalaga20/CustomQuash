STUDENT_ID=3082970

quash: quash.c
	gcc -g $^ -o $@

clean:
	-rm quash

run:
	./quash
