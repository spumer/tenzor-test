#pragma once

#include "trim.h"
#include <libxml/HTMLparser.h>
#include <string>
#include <vector>
#include <stack>
typedef std::stack<const xmlChar *, std::vector<const xmlChar*>> chars_stack_t;

//! TODO: Replace lazy* by filter class

inline void lazyFormat_Post(const xmlChar* tag, Node& target);
inline void lazyFormat_Pre(const xmlChar* tag, Node& target);

class Document {
	private:
		htmlParserCtxtPtr m_context;
		htmlSAXHandler m_saxHandler;
		char *m_htmlPage;
		size_t m_size;
		std::basic_string<xmlChar> m_url;			/* Current url from <a> */
		chars_stack_t m_levelStack;				/* Tag nested level */
		std::stack<Node, std::vector<Node>> m_parentStack;
	public:
		std::vector<Node> m_Tags;					/* Results will be stored here */
		std::vector<const xmlChar *> m_BlockTag;
		std::vector<const xmlChar *> m_LineTag;
	private:
		bool IsManaged(const xmlChar * target);
		bool IsAdditional(const xmlChar * target);
		bool IsValidTag();
	public:
		Document(char *page, size_t size);
		~Document();
		void parse();
		static bool IsLink(const xmlChar *tag);
		static void elementStarted(void * document, const xmlChar *name, const xmlChar ** atts);
		static void elementEnded(void * document, const xmlChar * name);
		static void dataRecieved(void * document, const xmlChar * ch, int len);
};


Document::Document(char *page, size_t size)
	: m_htmlPage(page), m_size(size)
{
	memset(&m_saxHandler, 0, sizeof(m_saxHandler));
	m_saxHandler.startElement = elementStarted;
	m_saxHandler.endElement = elementEnded;
	m_saxHandler.characters	= dataRecieved;
	m_saxHandler.cdataBlock = dataRecieved;
	
	m_context = htmlCreatePushParserCtxt(&m_saxHandler, this, m_htmlPage, m_size, ""/* filename */, XML_CHAR_ENCODING_UTF8);
}

Document::~Document() {
	xmlFreeDoc(m_context->myDoc);
	xmlFreeParserCtxt(m_context);
}

void Document::parse() {
	htmlParseChunk(m_context, m_htmlPage, m_size, 0);
	htmlParseChunk(m_context, "", 0, 1);
	htmlCtxtReset(m_context);
}

void Document::elementStarted(void * document, const xmlChar * name, const xmlChar ** atts) {
	Document *pDoc = (Document *)document;
	pDoc->m_levelStack.push(name);
	
	if(pDoc->IsManaged(name)) pDoc->m_parentStack.emplace(name);
	else if( !pDoc->m_parentStack.empty() ) {
		if(pDoc->IsAdditional(name))
			lazyFormat_Pre(name, pDoc->m_parentStack.top());
		else
			pDoc->m_parentStack.top().m_Level++;
	}
	
	/* Save link address */
	if(IsLink(name)) {
		pDoc->m_url.clear();
		while(atts != NULL && atts[0] != NULL) {
			if( !xmlStrcmp(atts[0], (const xmlChar *)"href") ) {
				pDoc->m_url += '[';
				pDoc->m_url += (++atts)[0];
				pDoc->m_url += ']';
				break;
			}
			++atts;
		}
	}
}

void Document::elementEnded(void * document, const xmlChar * name) {
	Document *pDoc = (Document *)document;
	if(pDoc->m_parentStack.empty()) return;
		
	if( pDoc->IsManaged(name) ) {
		if( (pDoc->m_parentStack.top()).size() > 0 )
			pDoc->m_Tags.emplace_back( pDoc->m_parentStack.top() );
		pDoc->m_parentStack.pop();
	} else if( !pDoc->IsAdditional(name) ) pDoc->m_parentStack.top().m_Level--;
	lazyFormat_Post(name, pDoc->m_parentStack.top());
	pDoc->m_levelStack.pop();
}

void Document::dataRecieved(void * document, const xmlChar * ch, int len) {
	Document *pDoc = (Document *)document;
	if( pDoc->IsValidTag() && !pDoc->m_parentStack.empty() && pDoc->m_parentStack.top().m_Level <= 1) {
		xmlChar *trimStr = (xmlChar *)trim( (const char *)ch );
		if(trimStr != NULL) { 
			if(trimStr[0] > 64) (pDoc->m_parentStack.top()).m_Content.append( {' '} );	// small ASCII hack prevent missing space after trim (usually for <a>)
			if( IsLink(pDoc->m_levelStack.top()) ) {
				pDoc->m_parentStack.top().m_Links += strlen((char*)trimStr);
				/* insert link as [%s] */
				if(pDoc->m_url.size() > 2) {	// 2 is '[' + ']'
					(pDoc->m_parentStack.top()).m_Content.append( pDoc->m_url );
					(pDoc->m_parentStack.top()).m_Links += pDoc->m_url.size();
				}
			}
			(pDoc->m_parentStack.top()).m_Content.append(trimStr);
			free(trimStr);
		}
	}
}

bool Document::IsManaged(const xmlChar * target) {
	for( auto tag : m_BlockTag )
		if( !xmlStrcmp(tag, target) )
			return true;
	return false;
}

bool Document::IsAdditional(const xmlChar * target) {
	for( auto tag : m_LineTag )
		if( !xmlStrcmp(tag, target) )
			return true;
	return false;
}

/**
*	Inlines
*/

inline void lazyFormat_Post(const xmlChar* tag, Node& target) {
	if( !xmlStrcmp(tag, (const xmlChar *)"p") || !xmlStrcmp(tag, (const xmlChar *)"h1") || !xmlStrcmp(tag, (const xmlChar *)"h2") || !xmlStrcmp(tag, (const xmlChar *)"h3") || !xmlStrcmp(tag, (const xmlChar *)"h4") ) 
		{ target.m_Content.append( {'\n', '\n'} ); }
	else if(!xmlStrcmp(tag, (const xmlChar *)"br") || !xmlStrcmp(tag, (const xmlChar *)"ul") || !xmlStrcmp(tag, (const xmlChar *)"ol") || !xmlStrcmp(tag, (const xmlChar *)"li") 
	  )
		{ target.m_Content.append( {'\n'} ); }
	
}

inline void lazyFormat_Pre(const xmlChar* tag, Node& target) {
	if(!xmlStrcmp(tag, (const xmlChar *)"li"))
		target.m_Content.append( {'\t', '*'} );
}

inline bool Document::IsValidTag() {
	const xmlChar * tag = m_levelStack.top();
	return IsManaged(tag) || IsAdditional(tag);
}

inline bool Document::IsLink(const xmlChar *tag) { return !xmlStrcmp(tag, (const xmlChar *)"a"); }
