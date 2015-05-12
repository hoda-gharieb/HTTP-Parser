#include <stdio.h>
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <cstring>
#include <direct.h>
#include <tchar.h>
#include <urlmon.h>

using namespace std;

#pragma comment(lib, "urlmon.lib") //urlmon library
#pragma comment(lib,"ws2_32.lib") //Winsock Library

char vect[100000] = { 0 };

//---------- return the string of a given number -----------//
string getStr(int n)
{
	stringstream sin;
	sin << n << endl;
	string s;
	sin >> s;
	return s;
}

//--------------- cutting html into tokens ------------------------//
void getTags()
{
	int index;

	//===== getting all data in html_source string =====//
	ifstream fin;
	fin.open("rawData.txt");
	string html_source = "", s;
	while (getline(fin, s))
		html_source += s;
	fin.close();

	ofstream fout;
	fout.open("tags.txt");

	//========== carry out tokenizing of the code ==========//
	while (html_source.size() != 0)
	{
		if (html_source.find('<') == -1)
		{
			fout << html_source << endl;
			html_source = "";
		}
		else if (html_source[0] == '<')
		{
			index = html_source.find('>');
			if (index == -1)
			{
				html_source = "";
				continue;
			}
			fout << html_source.substr(0, index + 1) << endl;;
			html_source = html_source.erase(0, index + 1);
		}
		else
		{
			index = html_source.find('<');
			fout << html_source.substr(0, index) << endl;
			html_source = html_source.erase(0, index);
		}
	}

	fout.close();
}

//-------------- generate script, css files and main html page -------------//
void generate_page()
{
	mkdir("output\\myfolder");
	mkdir("output\\page");
	int scpt = 0, img = 0, css = 0;

	ifstream fin;
	fin.open("tags.txt");
	string tag;
	ofstream hout;
	hout.open("output\\final.htm");


	while (getline(fin, tag))
	{
		if (tag.find("<script") == 0)//== found a new script
		{
			ofstream fout;
			string s = "output\\page\\" + getStr(scpt++) + ".js";
			string ss = "page\\" + getStr(scpt) + ".js";
			fout.open(s);
			fout << tag << endl;
			while (getline(fin, tag))
			{
				fout << tag << endl;
				if (tag == "</script>")
					break;
			}
			fout.close();
			tag = "<script src = \"" + ss + "\">< / script>";
			hout << tag << endl;
		}
		else if (tag.find("<img") == 0)//== found a new image
		{
			if (tag.find("src=\"http") != -1)
			{
				int st = tag.find("http");
				int e = tag.find('\"', st);
				string lnk = tag.substr(st, e - st);
				char src[1024], dst[1024];
				strcpy(src, lnk.c_str());
				lnk = "output\\myfolder\\" + getStr(img++) + ".jpg";
				strcpy(dst, lnk.c_str());
				tag = tag.erase(st, e - st);
				lnk = "myfolder\\" + getStr(img) + ".jpg";
				tag = tag.insert(st, lnk);
				HRESULT hr = URLDownloadToFile(NULL, src, dst, 0, NULL);//=== downloading the image
				hout << tag << endl;
			}
		}
		else if (tag == "<style type=\"text/css\">")//=== found the css style code
		{
			ofstream fout;
			string s = "output\\page\\" + getStr(css++) + ".css";
			string ss = "page\\" + getStr(css) + ".css";
			fout.open(s);
			fout << tag << endl;
			while (getline(fin, tag))
			{
				fout << tag << endl;
				if (tag == "</style>")
					break;
			}
			fout.close();
			tag = "<head>\n<link rel=\"stylesheet\" type=\"text/css\" href=\"" + ss + "\">\n</head>";
			hout << tag << endl;
		}
		else
			hout << tag << endl;
	}
	hout.close();
	fin.close();
}

//========== mkdir("output\\myfolder"); This command can be used to make a new directory if it doesn't exist

int main(int argc, _TCHAR *argv[])
{
	WSADATA wsa;
	//char *hostname = "www.shorouknews.com";
	char *hostname = new char();
	char ip[100];
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	string site;

	cout << "**=== Press 0 for get 1 for post ===**" << endl;
	int num;
	cin >> num;
	
	if (num == 1)
	{
		site = "www.post.com";
	}
	else
	{
		cout << "**=== Enter the site ===**" << endl;
		cin >> site;		
	}

	strcpy(hostname, site.c_str());
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	if ((he = gethostbyname(hostname)) == NULL)
	{
		printf("gethostbyname failed : %d", WSAGetLastError()); //gethostbyname failed
		return 1;
	}

	//====Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
	addr_list = (struct in_addr **) he->h_addr_list;

	for (i = 0; addr_list[i] != NULL; i++)
		strcpy(ip, inet_ntoa(*addr_list[i]));//Return the first one;
	printf("%s resolved to : %s\n", hostname, ip);


	WSAData version;
	WORD mkword = MAKEWORD(2, 2);
	int what = WSAStartup(mkword, &version);

	if (what != 0)
		std::cout << "This version is not supported! - \n" << WSAGetLastError() << std::endl;
	else
		std::cout << "Good - Everything fine!\n" << std::endl;

	SOCKET u_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (u_sock == INVALID_SOCKET)
		std::cout << "Creating socket fail\n";
	else
		std::cout << "It was okay to create the socket\n";

	//Socket address information
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(80);//http port --https -->23


	//========== Addressing finished ==========//

	//=== connecting
	int conn = connect(u_sock, (SOCKADDR*)&addr, sizeof(addr));
	if (conn == SOCKET_ERROR)
	{
		cout << "Error - when connecting " << WSAGetLastError() << endl;
		closesocket(u_sock);
		WSACleanup();
	}

	//==== Send some message to remote host https 443 http 80 ( char* mymsg= "GET / HTTP/1.1 \r\n Host:www.youm7.com \r\n\r\n" ) ====//
	if (!num)
		site = "GET / HTTP/1.1\r\nHost: " + site + "\r\n\r\n";
	else
		site = "POST / HTTP/1.1\r\nHost: " + site + "\r\n\r\n";
	/*const char mymsg[] = 
		"GET / HTTP/1.1\r\n"
		"Host: www.youm7.com\r\n"
		"\r\n";*/
	int smsg = send(u_sock, site.c_str(), sizeof(site.c_str())*10, 0);
	if (smsg == SOCKET_ERROR)
	{
		cout << "Error: " << WSAGetLastError() << endl;
		WSACleanup();
	}

	if (num == 1)
		return 0;

	int get = 1;
	ofstream fout;
	fout.open("rawData.txt");

	//======== gathering data from the website
	while (get != 0)
	{
		get = recv(u_sock, vect, 100000, 0);
		if (get == SOCKET_ERROR)
			std::cout << "Error in Receiving: " << WSAGetLastError() << std::endl;
		if (get != -1)
			fout << vect;
		cout << get << endl;
	}

	fout.close();
	getTags();
	generate_page();

	/*ifstream fin;
	fin.open("try.txt");
	string tag;
	ofstream fout;
	fout.open("rawData.txt");
	string s = "";
	while (getline(fin, tag))
	s += tag;
	fout << s << endl;
	fin.close();
	fout.close();

	getTags();
	generate_page();*/

	//========= load the page in te browser =========//
	ShellExecute(NULL, "open", "output\\final.htm", NULL, NULL, SW_SHOWNORMAL);



	WSACleanup();
	return 0;

}