#pragma once

#include "tests.hpp"

#include "net/HttpConnection.hpp"

#define FAIL(test, _fd) { std::cout << RED "[FAIL] " test RESET "\n"; close(_fd); return; }
#define PASS(test, _fd) { std::cout << GREEN "[PASS] " test RESET "\n"; close(_fd); }

#define UFAIL(test) { std::cout << RED "[FAIL] " test RESET "\n"; return; }
#define UPASS(test) { std::cout << GREEN "[PASS] " test RESET "\n"; }

void test_HttpConnectionTests(void);
void test_HttpParserTests(void);