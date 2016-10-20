#include "stack.hpp"
#include <catch.hpp>
#include <iostream>
#include <fstream>

SCENARIO("Stack init", "[init]") {
	stack<int> Stack;
	REQUIRE(Stack.count() == 0);
}
SCENARIO("Stack push and top", "[push and top]") {
	stack<int> Stack;
	Stack.push(3);
	REQUIRE(Stack.top() == 3);
}
SCENARIO("Stack pop", "[pop]") {
	stack<int> Stack;
	for (int i = 0; i < 10; i++) {
		Stack.push(i);
	}
	Stack.pop();
	REQUIRE(Stack.count() == 9);
}

SCENARIO("Stack count", "[count]") {
	stack<int> Stack;
	for (int i = 0; i < 10; i++) {
		Stack.push(i);
	}
	REQUIRE(Stack.count() == 10);
}

SCENARIO("operator =", "[operator =]") {
	stack<int> Stack;
	Stack.push(221);
	stack<int> Stack_ = Stack;
	REQUIRE(Stack_.top() == 221);
}
SCENARIO("copy constructor", "[copy constructor]") {
	stack<int> Stack;
	Stack.push(221);
	stack<int> Stack_(Stack);
	REQUIRE(Stack_.top() == 221);
}
SCENARIO("empty", "[empty]") {
	stack<int> Stack;
	REQUIRE(Stack.empty());
}
