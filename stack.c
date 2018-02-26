#include <stdio.h>
#include <stdlib.h>

//Source: https://stackoverflow.com/questions/34183641/implementing-stack-using-dynamic-array

/* Define a C data type "SizeType" that is a signed integer with the same
number of bits as a pointer: it is suitable for array indexing up to any
size that fits in memory. The format string for "ptrdiff_t" is "%td". */
typedef ptrdiff_t               SizeType;

struct Stack {
	SizeType *arr;
	SizeType top, capacity, size;
};

struct Stack *createStack(SizeType capacity) {
	struct Stack *s = (struct Stack *)malloc(sizeof(struct Stack));
	s->arr = (SizeType *)malloc(sizeof(SizeType)*capacity);
	s->top = -1;
	s->capacity = capacity;
	s->size = 0;
	return s;
}

void destroyStack(struct Stack *s) {
	if (s != NULL) free(s->arr);
	free(s);
}

void doubleStack(struct Stack *s) {
	s->capacity = s->capacity * 2;
	s->arr = realloc(s->arr, sizeof(SizeType)*s->capacity);
	if (s->arr != NULL) {
		printf("Array doubling happened successfully!\n");
	}
	else {
		perror("realloc");
	}
}

int isFull(struct Stack *s) {
	return s->size == s->capacity;
}

void push(struct Stack *s, SizeType item) {
	if (isFull(s))
		doubleStack(s);
	s->arr[++(s->top)] = item;
	s->size++;
}

int isEmpty(struct Stack *s) {
	return s->size == 0;
}

SizeType getSize(struct Stack *s) {
	return s->size;
}

SizeType pop(struct Stack *s) {
	if (isEmpty(s)) {
		printf("Empty stack!\n");
		return -1;
	}

	SizeType item = s->arr[(s->top)--];
	s->size--;
	return item;
}

/*
int main(void) {
	struct Stack *s = createStack(2);
	push(s, 1);
	push(s, 2);
	push(s, 3);
	push(s, 4);
	push(s, 5);
	pop(s);
	pop(s);
	destroyStack(s);
	return 0;
}'*/