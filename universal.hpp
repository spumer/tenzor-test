#include "trim.h"
#include <cstring>
#include <stack>
typedef stack<const xmlChar *, vector<const xmlChar*>> chars_stack_t;

class Document {
	private:
		htmlParserCtxtPtr m_context;
		htmlSAXHandler m_saxHandler;
		char *m_htmlPage;
		size_t m_size;
		bool m_tagValid;
		chars_stack_t m_levelStack;				/* Tag nested level */
		stack<Node, vector<Node>> m_parentStack;
	public:
		vector<Node> m_Tags;					/* Results will be stored here */
		vector<const xmlChar *> m_BlockTag;
		vector<const xmlChar *> m_LineTag;
	private:
		bool IsManaged(const xmlChar * target);
		bool IsAdditional(const xmlChar * target);
	public:
		Document(char *page, size_t size);
		~Document();
		void parse();
		bool IsValidTag();
		static void elementStarted(void * document, const xmlChar *name, const xmlChar ** atts);
		static void elementEnded(void * document, const xmlChar * name);
		static void dataRecieved(void * document, const xmlChar * ch, int len);
};


Document::Document(char *page, size_t size)
	: m_htmlPage(page), m_size(size), m_tagValid(false)
{
	memset(&m_saxHandler, 0, sizeof(m_saxHandler));
	m_saxHandler.startElement = elementStarted;
	m_saxHandler.endElement = elementEnded;
	m_saxHandler.characters	= dataRecieved;
	
	m_context = htmlCreatePushParserCtxt(&m_saxHandler, this, m_htmlPage, m_size, ""/* filename */, XML_CHAR_ENCODING_NONE);
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
	else if( !pDoc->m_parentStack.empty() && !pDoc->IsAdditional(name) )
		pDoc->m_parentStack.top().m_Level++;
}

void Document::elementEnded(void * document, const xmlChar * name) {
	Document *pDoc = (Document *)document;
	if(pDoc->m_parentStack.empty()) return;
	
	if( pDoc->IsManaged(name) ) {
		if( (pDoc->m_parentStack.top()).size() > 0 )
			pDoc->m_Tags.emplace_back( pDoc->m_parentStack.top() );
		pDoc->m_parentStack.pop();
	} else if( !pDoc->IsAdditional(name) ) pDoc->m_parentStack.top().m_Level--;
	pDoc->m_levelStack.pop();
}

void Document::dataRecieved(void * document, const xmlChar * ch, int len) {
	Document *pDoc = (Document *)document;
	if( pDoc->IsValidTag() && !pDoc->m_parentStack.empty() && /*pDoc->m_levelStack.size() -*/ pDoc->m_parentStack.top().m_Level <= 1) {
		xmlChar *trimStr = (xmlChar *)trim( (const char *)ch );
		if(trimStr != NULL) {
			(pDoc->m_parentStack.top()).m_Content.append(trimStr);
			if( !xmlStrcmp(pDoc->m_levelStack.top(), (const xmlChar *)"a") ) {
				pDoc->m_parentStack.top().m_Links += strlen((char*)trimStr);
			}
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

inline bool Document::IsValidTag() {
	const xmlChar * tag = m_levelStack.top();
	return IsManaged(tag) || IsAdditional(tag);
}
