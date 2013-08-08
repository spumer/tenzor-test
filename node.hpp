#pragma once

#include <string>
#include <cstring>
#include <libxml/HTMLparser.h>

#define strdup(A) ((xmlChar *)strdup((const char *)(A)))

class Node {
	public:
		xmlChar *m_Name;
		size_t m_Links;
		size_t m_Level;
		std::basic_string<xmlChar> m_Content;
	
	Node(const xmlChar *tag) : m_Name(strdup(tag)), m_Links(0), m_Level(0) {}
	~Node() { if(m_Name) free(m_Name); }
	// Copy-semantic
	Node(const Node& orig) : m_Name( strdup(orig.m_Name) ), m_Content(orig.m_Content), m_Links(orig.m_Links), m_Level(orig.m_Level) {}
	Node& operator=(const Node& orig) noexcept {
		if(m_Name) free(m_Name);
		m_Name = strdup(orig.m_Name);
		m_Content = orig.m_Content;
		m_Links = orig.m_Links;
		m_Level = orig.m_Level;
		return *this;
	}
	// Move-semantic
	Node(Node&& old) : m_Name(old.m_Name), m_Content(std::move(old.m_Content)), m_Links(old.m_Links), m_Level(old.m_Level) { old.m_Name = NULL; }
	Node& operator=(Node&& old) noexcept {
		if(m_Name) free(m_Name);
		m_Name = old.m_Name;
		old.m_Name = NULL;
		m_Content = std::move(old.m_Content);
		m_Links = old.m_Links;
		m_Level = old.m_Level;
		return *this;
	}

	size_t size() const { return m_Content.size() - m_Links; }
};
