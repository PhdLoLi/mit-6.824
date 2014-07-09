#include <stdio.h>
#include <map>
#include <string>
#include <iostream>
using   namespace   std; 
main()
{
	int id;
	id = 1;
	map<int, string> lock;
	lock.insert(pair<int, string>(id,"locked"));
	printf("%d:%s\n",id,lock[id].c_str());
//	cout << id << " : " << lock[id] << endl; 	
	lock[id] = "free";
//	printf("%d:%s\n",id,lock[id]);
/*	int sum = 0, value;
	while(cin >> value)
	{
		sum += value;
	}
	cout << "Sum is: " <<sum << endl;
	*/
	enum xxstatus {OK, RETRY, ERR};
	typedef int status;
	status loli = RETRY;
	cout << "haha\
		nimei\
		lala\
		meme\n" << loli <<  endl;
	return 0;
}
