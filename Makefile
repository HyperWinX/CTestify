all:
	@gcc main_test.c -o tests
	@./tests
	@rm tests
