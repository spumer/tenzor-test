#include <curl/curl.h>
#include <cstdlib>
#include <libxml/HTMLparser.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#include "node.hpp"
#include "universal.hpp"


// #include <forward_list>

typedef struct mem {
	char *data;
	size_t size;
	~mem() { if(data) free(data); }
} mem_t;

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp);
// void parseNode(htmlDocPtr doc, htmlNodePtr node, int level);
// void parseNode(htmlDocPtr doc, htmlNodePtr node, int level, Node *parent);

int main(int argc, char *argv[]) {
	CURL *curl_handle;
	CURLcode result;
	
	mem_t response = { (char*)malloc(1), 0 };
	curl_global_init(CURL_GLOBAL_ALL);
	
	curl_handle = curl_easy_init();
	
	curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, reinterpret_cast<void *>(&response));
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "awesome-agent/666.0");
	curl_easy_setopt(curl_handle, CURLOPT_REFERER, "http://google.com/");
	
	result = curl_easy_perform(curl_handle);
	if(result != CURLE_OK) {
		printf("Error: %s\n", curl_easy_strerror(result));
	}
	else {
		char *url, *encode;
		long retCode;
		curl_easy_getinfo(curl_handle, CURLINFO_EFFECTIVE_URL, &url);
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &retCode);
		curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_TYPE, &encode);
		encode = strrchr(encode, '=');
		if(encode) encode += 1;
		
		Document parser(response.data, response.size);
		parser.m_BlockTag.push_back((const xmlChar *)"div");
		parser.m_LineTag.push_back((const xmlChar *)"p");
		parser.m_LineTag.push_back((const xmlChar *)"a");
		parser.m_LineTag.push_back((const xmlChar *)"ol");
		parser.m_LineTag.push_back((const xmlChar *)"ul");
		parser.m_LineTag.push_back((const xmlChar *)"li");
		parser.m_LineTag.push_back((const xmlChar *)"b");
		parser.m_LineTag.push_back((const xmlChar *)"em");
		parser.m_LineTag.push_back((const xmlChar *)"pre");
		parser.m_LineTag.push_back((const xmlChar *)"blockquote"); // habr quote block
		parser.parse();
		
		// for( auto tag : parser.m_Tags )
			// printf("Tag: %s (%zd bytes) with %zd links: %s\n", tag.m_Name, tag.size(), tag.m_Links, tag.m_Content.c_str());
		
		auto result = max_element(parser.m_Tags.begin(), parser.m_Tags.end(),
								  [](const Node& lhs, const Node& rhs) {
									 return (lhs.size() < rhs.size());
								  });
		printf("Max element size is %zu\n", result->size());
		printf("Content:\n%s\n", result->m_Content.c_str());
		
		/*htmlDocPtr docPtr = htmlReadMemory(response.data, response.size, NULL, encode, HTML_PARSE_NOERROR|HTML_PARSE_NOBLANKS); 
		htmlNodePtr cur = xmlDocGetRootElement(docPtr);
		
		g_managed.push_front((const xmlChar *)"div");
		g_additional.push_front((const xmlChar *)"p");
		g_additional.push_front((const xmlChar *)"a");
		g_additional.push_front((const xmlChar *)"li");
		g_additional.push_front((const xmlChar *)"b");
		g_additional.push_front((const xmlChar *)"em");
		
		parseNode(docPtr, cur->xmlChildrenNode, 0, NULL);
		
		auto result = max_element(g_tags.begin(), g_tags.end(),
								  [](const Node& lhs, const Node& rhs) {
									 return (lhs.size() < rhs.size());
								  });
		printf("Max element size is %zu\n", result->size());
		printf("Content:\n%s\n", result->m_Content.c_str());*/
		/*for( auto tag : g_tags )
			printf("Tag: %s(%p) (%zd bytes) with %zd links: %s\n", tag->name, tag.m_pData, tag.size(), tag.m_Links, tag.m_Content.c_str());
		*/
		// xmlFreeDoc(docPtr);
		
		printf("Latest URL: %s\nResponse code: %ld\nEncode: %s\n", url, retCode, encode);
	}
	
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
	
	return 0;
}

static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp){
	size_t realsize = size * nmemb;

	mem_t *response = reinterpret_cast<mem_t *>(userp);
	response->data = (char *)realloc(response->data, response->size + realsize + 1);
	if(response->data == NULL) {
		printf("Mem alloc error\n");
		return -1;
	}
	
	memcpy( &(response->data[response->size]), contents, realsize );
	response->size += realsize;
	response->data[response->size] = 0;
	
	return realsize;
}

/*
void parseNode(htmlDocPtr doc, htmlNodePtr node, int level) {
	static xmlChar *content = NULL;
	while(node != NULL) {*/
		/* skip blank tags */
//		if( node->name != NULL && /*node->xmlChildrenNode != NULL &&*/ !xmlNodeIsText(node) ) {
/*			content = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
			if(content != NULL) {
				printf("Content: %s;\t", content);
				xmlFree(content);
				if( !xmlStrcmp(node->name, (const xmlChar *)"a") ) {
					content = xmlGetProp(node, (const xmlChar *)"href");
					printf("[%s] ", content);
					xmlFree(content);
				}
			}
			printf("TAG: %s. Level: %d\n", node->name, level);
			if(node->xmlChildrenNode) parseNode(doc, node->xmlChildrenNode, level+1);
		}
		node = node->next;
	}
	return;
}

bool isManaged(htmlNodePtr target) {
	for( auto tag : g_managed )
		if( !xmlStrcmp(tag, target->name) )
			return true;
	return false;
}

bool isAdditional(htmlNodePtr target) {
	for( auto tag : g_additional )
		if( !xmlStrcmp(tag, target->name) )
			return true;
	return false;
}

void parseNode(htmlDocPtr doc, htmlNodePtr node, int level, Node *parent) {
	static xmlChar *content = NULL;
	Node *flowParent = parent;
	while(node != NULL) {*/
		/* skip blank tags */
/*		if( node->name != NULL && node->xmlChildrenNode != NULL && !xmlNodeIsText(node) ) {
			bool bManaged = isManaged(node);
			bool bAdditional = isAdditional(node);
			
			if(parent) {
				if(bAdditional) {
					if(level - parent->m_Level == 1) {
						content = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
						if(content != NULL) {
							//! possible seg fault on ubuntu libxml2 version.
							char *newStr = strdup((const char *)content);
                            parent->m_Content.append(content);
							free(newStr);
							xmlFree(content);
						}
						if(xmlChildElementCount(node) == 1) level -= 1;
					}
					// if( !xmlStrcmp(node->name, (const xmlChar *)"a") ) parent->m_Links++;
				}
			}*/
			
			// printf("%s with diff %ld\n", node->name, level - (parent ? parent->m_Level : 0L));
			// if(parent) {
			//	if(bAdditional) {
			//		content = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
			//		if(content != NULL) {
			//			parent->m_Content.append(content);
						/*if( !xmlStrcmp(node->name, (const xmlChar *)"p") ) {
							xmlChar *className = xmlGetProp(parent->m_pData, (const xmlChar *)"class");
							printf("P in DIV(%s) (level diff %ld. %p): %s\n", className, level - parent->m_Level, parent->m_pData, content);
							xmlFree(className);
						}*/
			//			xmlFree(content);
			//		}
			//		if( !xmlStrcmp(node->name, (const xmlChar *)"a") ) {
			//				parent->m_Links++;
			//		}
			//	}
			//} //else 
/*			if(bManaged) {
				g_tags.emplace_back(node);
				flowParent = &(g_tags.back());
				flowParent->m_Level = level;
			}
			// printf("TAG: %s. Level: %d\n", node->name, level);
			parseNode(doc, node->xmlChildrenNode, level+1, flowParent);
			flowParent = parent;
		}
		node = node->next;
	}
	return;
}*/