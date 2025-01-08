test_it: dependencies
	gcc test.c ../pcg-c-0.94/src/*.o src/*.o -o test -std=c99; ./test

dependencies: 
	cd src; gcc *.c -c -std=c99;

