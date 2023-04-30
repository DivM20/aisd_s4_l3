#include<iostream>
#include<vector>
#include <string>
#include <queue>
#include <unordered_map>
#include <map>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;

namespace General{
	ifstream get_dataset(){
		ifstream enwik8("enwik8_cut");
		return enwik8;
	}

	string file_to_str(ifstream &file) {
		string str;
		if (file) {
			ostringstream ss;
			ss << file.rdbuf(); // reading data
			str = ss.str();
		}
		return str;
	}

	void make_copy(fstream &fin, fstream& fout) {
		if (fout) {
			fin << fout.rdbuf(); // reading data
		}
	}
}

namespace Huffman {
	using namespace std;

	struct Node
	{
		char ch;
		int freq;
		Node* left, * right;
	};

	Node* getNode(char ch, int freq, Node* left, Node* right)
	{
		Node* node = new Node();

		node->ch = ch;
		node->freq = freq;
		node->left = left;
		node->right = right;

		return node;
	}

	struct comp
	{
		bool operator()(Node* l, Node* r)
		{
			return l->freq > r->freq;
		}
	};

	void encode(Node* root, string str, unordered_map<char, string>& huffmanCode)
	{
		if (root == nullptr)
			return;
		if (!root->left && !root->right) {
			huffmanCode[root->ch] = str;
		}

		encode(root->left, str + "0", huffmanCode);
		encode(root->right, str + "1", huffmanCode);
	}

	string get_encoded_text(string text)
	{
		//frequency map
		unordered_map<char, int> freq;
		for (char ch : text) {
			freq[ch]++;
		}

		priority_queue<Node*, vector<Node*>, comp> pq;

		for (auto pair : freq) {
			pq.push(getNode(pair.first, pair.second, nullptr, nullptr));
		}

		while (pq.size() != 1)
		{
			Node* left = pq.top(); pq.pop();
			Node* right = pq.top();	pq.pop();

			int sum = left->freq + right->freq;
			pq.push(getNode('\0', sum, left, right));
		}

		Node* root = pq.top();

		unordered_map<char, string> huffmanCode;
		encode(root, "", huffmanCode);

		string encoded_text = "";
		for (char ch : text) {
			encoded_text += huffmanCode[ch];
		}

		return encoded_text;
	}
}

namespace RLE {
	string get_encoded_text(string text) {
		string encoded_text = "";
		for (int i = 0; i < text.length(); i++) {
			int count = 1;

			while (text[i] == text[i + 1] && i < text.length() - 1) {
				count++;
				i++;
			}
			encoded_text += text[i];
			encoded_text += to_string(count);
		}
		return encoded_text;
	}
}

namespace LZ78 {

	struct Node {
		int index;
		string data;
		Node* next;
	};

	void st_Node(Node* head, int index, string data) {
		head->index = index;
		head->data = data;
		head->next = NULL;
	}

	void insert_Node(Node* head, int index, string data) {
		Node* new_Node = new Node;
		new_Node->index = index;
		new_Node->data = data;
		new_Node->next = NULL;

		Node* curr = head;
		while (curr != NULL)
		{
			if (curr->next == NULL)
			{
				curr->next = new_Node;
				return;
			}
			curr = curr->next;
		}
	}

	Node* search_Node(Node* head, string data)
	{
		Node* curr = head;
		while (curr != NULL)
		{
			if (data.compare(curr->data) == 0)
				return curr;
			else
				curr = curr->next;
		}
		return NULL;
	}

	Node* search_Node(Node* head, int index)
	{
		Node* curr = head;
		while (curr != NULL)
		{
			if (index == curr->index)
				return curr;
			else
				curr = curr->next;
		}
		return NULL;
	}

	bool delete_Node(Node* head, Node* to_delete) {
		if (to_delete == NULL)
			return false;
		else if (to_delete == head)
		{
			head = to_delete->next;
			delete to_delete;
			return true;
		}
		else {
			Node* curr = head;
			while (curr)
			{
				if (curr->next == to_delete)
				{
					curr->next = to_delete->next;
					delete to_delete;
					return true;
				}
				curr = curr->next;
			}
			return false;
		}
	}

	string get_encoded_text(string text)
	{
		Node* dictionary = new Node;
		string word, result;
		int length, last_seen, index = 1;

		length = (int)text.length();
		word = text[0];
		st_Node(dictionary, 1, word);
		result += "0," + word;

		for (int i = 1; i < length; i++)
		{
			string data;
			data = text[i];

		re_check:
			Node* search = search_Node(dictionary, data);

			if (search)
			{
				i++;
				data += text[i];
				last_seen = search->index;
				goto re_check;
			}
			else
			{
				char zero;
				if (text[i] == ' ')
					zero = '0';
				else
					zero = text[i];

				if ((int)data.length() < 2)
					result += " " + to_string(0) + "," + zero;
				else
					result += " " + to_string(last_seen) + "," + zero;

				index++;
				if (i != length)
					insert_Node(dictionary, index, data);
			}
		}

		return result;
	}
}

namespace BWT {
	
	const char EOT_CHAR = '^';

	string cyclic_shift(string text, int distance) {
		return text.substr(distance, string::npos) + text.substr(0, distance);
	}

	string get_encoded_text(string text) {
		text.append(&EOT_CHAR);

		vector<string> shifts;

		for (unsigned int i = 0; i < text.length(); i++) {
			string shift = cyclic_shift(text, i);

			shifts.push_back(shift);
		}

		sort(shifts.begin(), shifts.end());

		string encoded_text = "";

		for (vector<string>::iterator it = shifts.begin(); it != shifts.end(); it++) {
			encoded_text = encoded_text + it->at(it->length() - 1);
		}

		return encoded_text;
	}
}

namespace MTF {

	vector<int> get_v(string text) {
		vector<int> v;
		for (char c : text) {
			if (find(v.begin(), v.end(), c) == v.end()) {
				v.push_back(c);
			}
		}
		return v;
	}
	
	string get_encoded_text(string text, vector<int> v){
		string encoded_text = "";
		vector<int> res;
		for (char c : text) {
			int idx = find(v.begin(), v.end(), c) - v.begin();
			res.push_back(idx);
			v.erase(v.begin() + idx);
			v.insert(v.begin(), c);
		}
		for (int x : res) {
			encoded_text += to_string(x);
			encoded_text += " ";
		}
		return encoded_text;
	}
}

namespace AC {
	string get_encoded_text(string text) {
		map<char, int> freq;
		for (char c : text) {
			freq[c]++;
		}

		vector<pair<char, double>> probs;
		for (auto p : freq) {
			probs.emplace_back(p.first, p.second / static_cast<double>(text.size()));
		}

		double low = 0.0;
		double high = 1.0;
		double range = 1.0;

		for (char c : text) {
			double symbolLow, symbolHigh;
			for (auto p : probs) {
				if (p.first == c) {
					symbolLow = low;
					symbolHigh = low + range * p.second;
					break;
				}
				else {
					low += range * p.second;
				}
			}
			range = symbolHigh - symbolLow;
			low = symbolLow;
			high = symbolHigh;
		}

		double code = (low + high) / 2.0;
		return to_string(code);
	}
}

namespace PPM{
	
	const int Order = 4;
	const int size = Order + 1;

	map<string, int> char_freq;
	map<string, int> context_freq[size];

	void update_context(vector<string> &context, string nextChar)
	{
		context.erase(context.begin());
		context.push_back(nextChar);
	}

	void update_counts(vector<string> &context, string nextChar)
	{
		char_freq[nextChar]++;

		for (int i = 0; i < size; i++) {
			if (i == context.size()) {
				break;
			}

			string context_str;
			for (int j = 0; j < i; j++) {
				context_str += context[context.size() - i + j];
			}
			context_str += nextChar;

			context_freq[i][context_str]++;
		}
	}

	void get_encoded_text(ifstream &fin,ofstream &fout){;
		vector<string> context(Order, "");
		string output = "";
		string line;

		while (getline(fin, line)) {
			string output = "";

			for (char it : line) {
				string next_char = "";
				next_char += it;
				int order = Order;
				while (order >= 0) {
					string context_str;
					for (int j = 0; j < order; j++) {
						context_str += context[context.size() - order + j];
					}
					context_str += next_char;

					if (context_freq[order][context_str] > 0) {
						//matching content has been found
						output += to_string(context_freq[order][context_str]);
						output += context_str.substr(order, 1);
						update_context(context, next_char);
						update_counts(context, next_char);
						break;
					}
					else {
						order--;
					}
				}

				if (order < 0) {
					output += "0";
					output += next_char;
					update_context(context, next_char);
					update_counts(context, next_char);
				}
			}
			fout << output << " ";
		}
		fin.close();
	}
}

int main() {
	ifstream data = General::get_dataset();
	string s = General::file_to_str(data);


	
	int start_len, end_len;
	start_len = s.length();

	string temp_s;


	//cout << "\n-----------------------------";
	//cout << "\n  Test 1\n";
	//
	//auto start_test = chrono::steady_clock::now();
	//temp_s = Huffman::get_encoded_text(s);
	//auto end_test = chrono::steady_clock::now();
	//cout << "\nTime: " << chrono::duration_cast<chrono::seconds>(end_test - start_test).count();
	//end_len = temp_s.length();
	//cout << "\nEfficency: " << double(start_len) / double(end_len) << endl;


	//cout << "\n-----------------------------";
	//cout << "\n  Test 2\n";
	//
	//auto start_test = chrono::steady_clock::now();
	//temp_s = AC::get_encoded_text(s);
	//auto end_test = chrono::steady_clock::now();
	//cout << "\nTime: " << chrono::duration_cast<chrono::milliseconds>(end_test - start_test).count();
	//end_len = temp_s.length();
	//cout << "\nEfficency: " << (double(start_len) / double(end_len))*8 << endl;


	//cout << "\n-----------------------------";
	//cout << "\n  Test 3\n";
	//
	//auto start_test = chrono::steady_clock::now();
	//temp_s = LZ78::get_encoded_text(s);
	//auto end_test = chrono::steady_clock::now();
	//cout << "\nTime: " << chrono::duration_cast<chrono::seconds>(end_test - start_test).count();
	//end_len = temp_s.length();
	//
	//cout << "\nEfficency: " << double(start_len) / double(end_len) << endl;


	//cout << "\n-----------------------------";
	//cout << "\n  Test 4\n";

	//vector<int> v = MTF::get_v(BWT::get_encoded_text(s));
	//auto start_test = chrono::steady_clock::now();
	//temp_s = Huffman::get_encoded_text(MTF::get_encoded_text(BWT::get_encoded_text(s), v));
	//auto end_test = chrono::steady_clock::now();
	//cout << "\nTime: " << chrono::duration_cast<chrono::seconds>(end_test - start_test).count();
	//end_len = temp_s.length();

	//cout << "\nEfficency: " << (double(start_len) / double(end_len))*8 << endl;

	//cout << "\n-----------------------------";
	//cout << "\n  Test 5\n";

	//vector<int> v = MTF::get_v(BWT::get_encoded_text(s));
	//auto start_test = chrono::steady_clock::now();
	//temp_s = AC::get_encoded_text(MTF::get_encoded_text(BWT::get_encoded_text(s),v));
	//auto end_test = chrono::steady_clock::now();
	//cout << "\nTime: " << chrono::duration_cast<chrono::seconds>(end_test - start_test).count();
	//end_len = temp_s.length();

	//cout << "\nEfficency: " << double(start_len) / double(end_len) << endl;

	//cout << "\n-----------------------------";
	//cout << "\n  Test 6\n";

	//vector<int> v = MTF::get_v((BWT::get_encoded_text(RLE::get_encoded_text(s))));
	//auto start_test = chrono::steady_clock::now();
	//temp_s = Huffman::get_encoded_text(RLE::get_encoded_text(MTF::get_encoded_text(BWT::get_encoded_text(RLE::get_encoded_text(s)),v)));
	//auto end_test = chrono::steady_clock::now();
	//cout << "\nTime: " << chrono::duration_cast<chrono::seconds>(end_test - start_test).count();
	//end_len = temp_s.length();

	//cout << "\nEfficency: " << (double(start_len) / double(end_len))*8 << endl;

	//cout << "\n-----------------------------";
	//cout << "\n  Test 7\n";

	//vector<int> v = MTF::get_v((BWT::get_encoded_text(RLE::get_encoded_text(s))));
	//auto start_test = chrono::steady_clock::now();
	//temp_s = AC::get_encoded_text(RLE::get_encoded_text(MTF::get_encoded_text(BWT::get_encoded_text(RLE::get_encoded_text(s)),v)));
	//auto end_test = chrono::steady_clock::now();
	//cout << "\nTime: " << chrono::duration_cast<chrono::seconds>(end_test - start_test).count();
	//end_len = temp_s.length();
	//cout << "\nEfficency: " << double(start_len) / double(end_len) << endl;


	//cout << "\n-----------------------------";
	//cout << "\n  Test 8\n";

	ifstream in("enwik8_cut");
	ofstream out("out.txt");
	auto start_test = chrono::steady_clock::now();
	PPM::get_encoded_text(in, out);
	auto end_test = chrono::steady_clock::now();
	out.close();
	ifstream ins("out.txt");
	temp_s = General::file_to_str(ins);
	cout << "\nTime: " << chrono::duration_cast<chrono::seconds>(end_test - start_test).count();
	end_len = temp_s.length();

	cout << "\nEfficency: " << double(start_len) / double(end_len) << endl;


	return 0;
}