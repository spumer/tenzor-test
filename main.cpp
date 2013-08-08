#include <algorithm>
#include <cstdio>

#include "node.hpp"
#include "universal.hpp"
#include "connection.hpp"
#include "utf8.h"

using namespace std;

//! TODO: itegrate utf8cpp with Document class

void fix_utf8_string(basic_string<xmlChar>& str)
{
    basic_string<xmlChar> temp;
    utf8::replace_invalid(str.begin(), str.end(), back_inserter(temp));
    str = temp;
}

int main(int argc, char *argv[]) {	
	CURLConnect::InitCurl();
	
	CURLConnect conn;
	conn.setUrl(argv[1]);
	mem_t response = conn.getData();
	if(conn.hasError()) {
		return -1;		//! TODO: replace by error from enum or something like error_code.
	}
	
	Document parser(response.data, response.size);
	parser.m_BlockTag.push_back((const xmlChar *)"div");
	parser.m_LineTag.insert(parser.m_LineTag.end(), 
	{
		(const xmlChar *)"p",
		(const xmlChar *)"a",
		(const xmlChar *)"h1",
		(const xmlChar *)"h2",
		(const xmlChar *)"h3",
		(const xmlChar *)"h4",
		(const xmlChar *)"ol",
		(const xmlChar *)"ul",
		(const xmlChar *)"li",
		(const xmlChar *)"b",
		(const xmlChar *)"i",
		(const xmlChar *)"em",
		(const xmlChar *)"pre",
		(const xmlChar *)"blockquote" // habr quote block
	});
	parser.parse();
	
	auto result = std::max_element(parser.m_Tags.begin(), parser.m_Tags.end(),
								   [](const Node& lhs, const Node& rhs) {
									  return (lhs.size() < rhs.size());
									});
									
	// printf("Max element size is %zu\n", result->size());
	// printf("Content:\n%s\n", result->m_Content.c_str());
	// printf("Latest URL: %s\nResponse code: %ld\nEncode: %s\n", url, retCode, encode);
	
	basic_string<xmlChar>& article = result->m_Content;
	fix_utf8_string(article);
	
	const xmlChar* str = article.c_str();
	size_t len = article.size();
	// info get from http://utfcpp.sourceforge.net
	utf8::iterator<const xmlChar*> u8_it(str, str, str + len);
	utf8::iterator<const xmlChar*> u8_end(str + len, str, str + len);
	utf8::iterator<const xmlChar*> u8_word = u8_it;
	
	
	size_t lineLen = 0;
	for(; u8_it != u8_end; ++lineLen, ++u8_it) {
		switch(*u8_it) {
			case ' ':
				u8_word = u8_it;
				break;
			case '\r':
			case '\n':
				lineLen = 0;
				break;
		}
		if(lineLen == 80) {
			(*(xmlChar*)u8_word.base()) = '\n';
			lineLen = 0;
		}
	}
	
	FILE *file = fopen("article.content", "w+");
	fprintf(file, "%s", str);
	fclose(file);	
	
	long code = conn.retCode();
	
	CURLConnect::CleanCurl();
	return code;
}


