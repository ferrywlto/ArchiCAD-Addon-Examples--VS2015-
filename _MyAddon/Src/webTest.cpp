#include "webTest.hpp"
#include <curl/curl.h>


struct MemoryStruct {
	char *memory;
	size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

void webTest()
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if (curl) {
		struct curl_slist *headers = NULL; /* init to NULL is important */
		headers = curl_slist_append(headers, "Accept: application/json");
		curl_easy_setopt(curl, CURLOPT_VERBOSE, headers);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "request=bar");
		curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.0.88:5000/api/File/Search");

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		res = curl_easy_perform(curl);
		curl_slist_free_all(headers); /* free the header list */
		curl_easy_cleanup(curl);
	}
	return;
}