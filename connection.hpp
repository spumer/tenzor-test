#pragma once

#include <curl/curl.h>
#include <cstdlib>

typedef struct {
	char *data = NULL;
	size_t size = 0;
} mem_t;

class HTTPConnection {
	public:
		virtual mem_t getData() = 0;		/* Get connection data (in our case is html page content) */
		virtual long retCode() const = 0;	/* Get return code. 200, 404, 503 etc. */
		virtual bool hasError() const = 0;	/* Exmpl: return retCode() != 200 */
		virtual void setUrl(const char *url) = 0;
		virtual ~HTTPConnection() = default;
};

class CURLConnect : public HTTPConnection {
	private:
		long m_retCode;					/* HTTP code store here */
		CURL *m_handle;
		CURLcode m_result;				/* Operation result code store here */
		mem_t m_buffer;
	public:
		CURLConnect();
		~CURLConnect();
		static void InitCurl();
		static void CleanCurl();
		static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) noexcept;
		
		// implement HTTPConnection interface
		void setUrl(const char *url) override;
		bool hasError() const override;
		long retCode() const override;
		mem_t getData() override;
};

CURLConnect::CURLConnect()
	: m_handle(curl_easy_init()), m_retCode(0), m_result(CURLE_OK)
{
	curl_easy_setopt(m_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
	curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_handle, CURLOPT_USERAGENT, "awesome-agent/666.0");
	curl_easy_setopt(m_handle, CURLOPT_REFERER, "http://google.com/");
}

CURLConnect::~CURLConnect() {
	curl_easy_cleanup(m_handle);
	if(m_buffer.data) free(m_buffer.data);
}

void CURLConnect::setUrl(const char *url) {
	curl_easy_setopt(m_handle, CURLOPT_URL, url);
}

#define response (pThis->m_buffer)
size_t CURLConnect::write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) noexcept {
	size_t realsize = size * nmemb;
	CURLConnect *pThis = (CURLConnect *)userp;

	response.data = (char *)realloc(response.data, response.size + realsize + 1);
	if(response.data == NULL) {
		return -1;	// set curl to error state
	}
	
	memcpy( response.data + response.size, contents, realsize );
	response.size += realsize;
	response.data[response.size] = 0;
	
	return realsize;
}
#undef response

bool CURLConnect::hasError() const {
	return m_result != CURLE_OK || m_retCode != 200;
}

long CURLConnect::retCode() const {
	return m_retCode;
}

mem_t CURLConnect::getData() {
	m_result = curl_easy_perform(m_handle);
	if(m_result != CURLE_OK) { mem_t empty; return empty; }
	
	//! TODO: get encoding from meta tag if in content-type not have info
	// curl_easy_getinfo(m_handle, CURLINFO_CONTENT_TYPE, &encode);
	// encode = strrchr(encode, '=');
	// if(encode) encode += 1;
	
	curl_easy_getinfo(m_handle, CURLINFO_RESPONSE_CODE, &m_retCode);
	return m_buffer;
}

/*
*	Remarks:
*	This init/cleanup function should called once for the program. 
*	Be careful when cleanup curl, it can be used by other parts of program.
*	Use it only when you strongly sure in your action.
*/
void CURLConnect::InitCurl() {
	curl_global_init(CURL_GLOBAL_ALL);
}

void CURLConnect::CleanCurl() {
	curl_global_cleanup();
}
