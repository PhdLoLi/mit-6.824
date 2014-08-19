#include <sstream>
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string text = "nali|12435|al|8979532|ad|3897895742|";
	stringstream ss(text);
	string sub_str;
	while(getline(ss,sub_str,'|'))
	{
	//	if(sub_str.size()!=0)
			if(sub_str == "al")
			{
				getline (ss,sub_str,'|');
				cout<<sub_str<<endl;
				break;
			}
	}
	sub_str += "OhOh";
	cout<<sub_str<<endl;
}

