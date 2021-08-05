#include <iostream>

#define STACK_MAX_SIZE 256
#define IGCT 8


//Build A Simple Interpreter
typedef enum {
	INT,
	TWIN
} ObjectType;

typedef struct sObject {
	ObjectType type;
	unsigned char marked;

	struct sObject* next;

	union {
		int value;

		struct {
			struct sObject* head;
			struct sObject* tail;
		};
	};
} Object;

//build stack and mark it through the "linked list" 
typedef struct {
	Object* stack[STACK_MAX_SIZE];
	int stackSize;

	Object* firstObject;

	int numObjects;

	int maxObjects;
} VM;


void push(VM* vm, Object* value) {
	vm->stack[vm->stackSize++] = value;
}


Object* pop(VM* vm) {
	return vm->stack[--vm->stackSize];
}



VM* newVM() {
	VM* vm = new VM; 
    vm->stackSize = 0;
	vm->firstObject = NULL;
	vm->numObjects = 0;
	vm->maxObjects = IGCT;
	return vm;
}


void mark(Object* object) {
	if (object->marked) return;

	object->marked = 1;

	if (object->type == TWIN) {
		mark(object->head);
		mark(object->tail);
	}
}

void markAll(VM* vm)
{
	for (int i = 0; i < vm->stackSize; i++) {
		mark(vm->stack[i]);
	}
}
//Finding and deleting an object
void markspeep(VM* vm)
{
	Object** object = &vm->firstObject;
	while (*object) {
		if (!(*object)->marked) {
			Object* unreached = *object;

			*object = unreached->next;
            delete[] unreached;

			vm->numObjects--;
		}
		else {
			(*object)->marked = 0;
			object = &(*object)->next;
		}
	}
}


void gc(VM* vm) {
	int numObjects = vm->numObjects;

	markAll(vm);
	markspeep(vm);

	vm->maxObjects = vm->numObjects * 2;

	std::cout<<"Collected "<< numObjects - vm->numObjects <<" objects, "<< vm->numObjects <<" left.\n";
}

Object* newObject(VM* vm, ObjectType type) {
	if (vm->numObjects == vm->maxObjects) gc(vm);

	Object* object = new Object; 
    object->type = type;
	object->next = vm->firstObject;
	vm->firstObject = object;
	object->marked = 0;

	vm->numObjects++;

	return object;
}

void pushInt(VM* vm, int intValue) {
	Object* object = newObject(vm, INT);
	object->value = intValue;

	push(vm, object);
}

Object* pushPair(VM* vm) {
	Object* object = newObject(vm, TWIN);
	object->tail = pop(vm);
	object->head = pop(vm);

	push(vm, object);
	return object;
}

void objectPrint(Object* object) {
	switch (object->type) {
		case INT:
		std::cout<<object->value;
        break;

		case TWIN:
		//printf("(");
        std::cout<<"(";
		objectPrint(object->head);
		//printf(", ");
        std::cout<<", ";
		objectPrint(object->tail);
		//printf(")");
		std::cout<<")";
        break;
	}
}

void freeVM(VM *vm) {
	vm->stackSize = 0;
	gc(vm);
	delete[] vm;
}

void first_test() {
	
    VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);

	gc(vm);
	freeVM(vm);
}

void second_test() {
	std::cout<<"2: Unreached objects are collected.\n";
    VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pop(vm);
	pop(vm);

	gc(vm);
	freeVM(vm);
}

void third_test() {
	std::cout<<"3: Reach the nested objects.\n";
    VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	pushPair(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	pushPair(vm);
	pushPair(vm);

	gc(vm);
	freeVM(vm);
}

void another_test() {
    std::cout<<"4: Cycles.\n";
	VM* vm = newVM();
	pushInt(vm, 1);
	pushInt(vm, 2);
	Object* a = pushPair(vm);
	pushInt(vm, 3);
	pushInt(vm, 4);
	Object* b = pushPair(vm);

	a->tail = b;
	b->tail = a;

	gc(vm);
	freeVM(vm);
}

void performance() {
	std::cout<<"Performance of GC.\n";
    VM* vm = newVM();

	for (int i = 0; i < 1000; i++) {
		for (int j = 0; j < 20; j++) {
			pushInt(vm, i);
		}

		for (int k = 0; k < 20; k++) {
			pop(vm);
		}
	}
	freeVM(vm);
}

int main(int argc, const char** argv) {
	first_test();
	second_test();
	third_test();
	another_test();
	performance();

	return 0;
}