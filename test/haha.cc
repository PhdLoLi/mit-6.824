#include <string>
#include <iostream>
using namespace std;
int main()
{
	std::string name;
	name = string("haha") + string(3,'\0') + string("lala") + string(5,'\0');
	std::string name2 = name;
	std::string name3 = string(3,'\0');
	std::cout<<name<<name.size()<<name2.size()<<name3.size()<<std::endl;
	
	return 0;
}
