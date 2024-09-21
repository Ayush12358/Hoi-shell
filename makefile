main:
	sudo apt-get install libcurl4-openssl-dev
	gcc main.c -lcurl && ./a.out