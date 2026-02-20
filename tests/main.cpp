#include "net.hpp"
#include <iostream>

int main()
{
	std::cout << "============== HttpConnection class ===============\n" << std::endl;
	test_HttpConnectionTests();
	
	std::cout << "============== HttpParser class ===============\n" << std::endl;
	test_HttpParserTests();
}