#include"threadpool.cpp"
#include<iostream>

using namespace std;
int func(int j) {
	for (int i = 0; i < j; i++)
	{
		cout << this_thread::get_id << " " << i <<"func1 " << endl;
		this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	return 10;
}
void func2(int j) {
	for (int i = 0; i < j; i++)
	{
		cout << this_thread::get_id << " " << i <<"func2" << endl;
		this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}
void func3() {
	for (int i = 0; i < 10; i++)
	{
		cout << this_thread::get_id << " " << i << "func3" << endl;
		this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}
void func4(string& c) {
	cout << c <<"func4"<< endl;
}
void func5(string&& c) {
	cout << c << "func5" << endl;
}
class A {
public:
	static void func(int index) {
		
		for (int i = 0; i < index; i++)
		{
			this_thread::sleep_for(std::chrono::milliseconds(50));
			cout << i << endl;
		}
	}
	void func2(int& index) {
		for (int i = 0; i < index; i++)
		{
			this_thread::sleep_for(std::chrono::milliseconds(50));
			cout << i << endl;
		}
	}
	/*void func3(int& index) {
	
	}*/
	void func4(int&& index) {
		for (int i = 0; i < index; i++)
		{
			this_thread::sleep_for(std::chrono::milliseconds(50));
			cout << i <<"func4"<< endl;
		}
	}

};
void func6(int&& index) {
	cout << index << endl;
}
void func6(int& index) {
	cout << index <<"func6r" << endl;
}
int main() {
	{threadpool tp(4);
	string s = "ssssss";
	string& sr = s;
	string sc = move(sr);
	string&& srr = "sss";
	cout << sc << endl;
	/*cout << sr << endl;
	func5(move(sr));
	func5(forward<string&&>(srr));*/
	}
	
	system("pause");
	return 0;
}