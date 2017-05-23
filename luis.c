#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <time.h>
#include <curl/curl.h>
//#include <uuid/uuid.h>
#include <sys/stat.h>

int libcurl_get(const char *url, std::vector<std::string> &header, std::string &buffer);
int libcurl_post(const char *url,  std::vector<std::string> &header, const char *data, long size, std::string &buffer);

struct timeval start, finish;
void timing_start()
{
	gettimeofday(&start, NULL);
}

double timing_end()
{
	double duration;
	
	gettimeofday(&finish, NULL);
	duration = finish.tv_sec - start.tv_sec + (finish.tv_usec - start.tv_usec)/1000000.0;

	return duration;
}

#if 0
std::string newGUID()
{
	uuid_t uu;

	uuid_generate(uu);

	return (char *)uu;
}
#endif

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("parameter error\n");
		return -1;
	}
	#if 0
	//const char *url = "https://westus.api.cognitive.microsoft.com/luis/v2.0/apps/3f8da0cb-ba35-4bbb-b2ed-e729f9cd30c4?subscription-key=f0489f68e448443582d3b3f610dcd38f&timezoneOffset=0&verbose=true&q=%e5%bc%80%e5%a7%8b%e6%89%93%e5%8d%b0";
	const char *token_url = "https://api.cognitive.microsoft.com/sts/v1.0/issueToken";
	header.push_back("Ocp-Apim-Subscription-Key:75d7c7aa34954698bfedc65698cf5521");
	//header.push_back("Content-Type: application/json");
	//header.push_back("subscription-key=f0489f68e448443582d3b3f610dcd38f");
	//header.push_back("timezoneOffset=0");
	//header.push_back("verbose=true");
	//header.push_back("q=a");
	//header.push_back("Connection: keep-alive");
	#endif

	#if 1
	curl_global_init(CURL_GLOBAL_ALL);

	std::string token_response;
	std::vector<std::string> token_header;
	const char *token_url = "https://api.cognitive.microsoft.com/sts/v1.0/issueToken";
	token_header.push_back("Ocp-Apim-Subscription-Key:75d7c7aa34954698bfedc65698cf5521");

	if (libcurl_post(token_url, token_header, NULL, 0, token_response))
	{
		printf("token:%s\n", token_response.c_str());
	}
	
	std::string pcm_response;
	std::vector<std::string> pcm_header;
	std::string pcm_url = "https://speech.platform.bing.com/speech/recognition/interactive/cognitiveservices/v1?language=zh-CN";
	pcm_url += "&format=json";
	pcm_url += "&requestid=39530efe-5677-416a-98b0-93e13ec93c2b";

	pcm_header.push_back("Host: speech.platform.bing.com");
	pcm_header.push_back("Accept: application/json;text/xml");
	pcm_header.push_back("ContentType: audio/wav; codec=""audio/pcm""; samplerate=16000");
	pcm_header.push_back("Authorization: Bearer " + token_response);

	char *postcontent;
	FILE *file;
	struct stat fileinfo;
	const char *filepath = argv[1];
	long readcount;
	long filesize;
	file = fopen(filepath, "rb");
	if (NULL == file)
	{
		goto exe_failed;
	}
	stat(filepath, &fileinfo);
	filesize = fileinfo.st_size;
	postcontent = (char *)malloc(filesize);
	if (NULL == postcontent)
	{
		goto exe_failed;
	}
	readcount = fread(postcontent, 1, filesize, file);
	if (readcount != filesize)
	{
		goto exe_failed;
	}

	if (libcurl_post(pcm_url.c_str(), pcm_header, NULL, 0, pcm_response))
	{
		printf("result:%s\n", pcm_response.c_str());
	}

exe_failed:
	if (NULL != file)
		fclose(file);
	if (NULL != postcontent)
		free(postcontent);
	
	curl_global_cleanup();
	#endif
}

int writer(char *data, size_t size, size_t nmemb, std::string *write_data)
{
	unsigned long sizes = size * nmemb;
	if (NULL == write_data)
	{
		return 0;
	}

	write_data->append(data, sizes);

	return sizes;
}

bool init(CURL *conn, const char *url, std::string *p_buffer)
{
	CURLcode code;

	if (NULL == conn)
	{
		return false;	
	}

	code = curl_easy_setopt(conn, CURLOPT_URL, url);
	if (CURLE_OK != code)
	{
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
	if (CURLE_OK != code)
	{
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, p_buffer);
	if (CURLE_OK != code)
	{
		return false;
	}

	return true;
}

int libcurl_get(const char *url, std::vector<std::string> &header, std::string &buffer)
{
	CURL *conn;
	CURLcode code;
	
	conn = curl_easy_init();

	if (!init(conn, url, &buffer))
	{
		return -1;
	}

	curl_slist *plist = NULL;
	for (std::vector<std::string>::iterator it = header.begin(); it != header.end(); it++)
	{
		plist = curl_slist_append(plist, it->c_str());
	}
	if (NULL != plist)
	{
		curl_easy_setopt(conn, CURLOPT_HTTPHEADER, plist);
	}
	
	code = curl_easy_perform(conn);
	if (CURLE_OK != code)
	{
		return -1;
	}

	curl_easy_cleanup(conn);

	return 1;
}

int libcurl_post(const char *url, std::vector<std::string> &header, const char *data, long size, std::string &buffer)
{
	CURL *conn;
	CURLcode code;
	
	conn = curl_easy_init();

	if (!init(conn, url, &buffer))
	{
		return -1;
	}
	
	curl_slist *plist = NULL;
	for (std::vector<std::string>::iterator it = header.begin(); it != header.end(); it++)
	{
		plist = curl_slist_append(plist, it->c_str());
	}
	if (NULL != plist)
	{
		curl_easy_setopt(conn, CURLOPT_HTTPHEADER, plist);
	}

	code = curl_easy_setopt(conn, CURLOPT_POST, true);
	if (CURLE_OK != code)
	{
		return -1;
	}

	if (NULL != data)
	{
		code = curl_easy_setopt(conn, CURLOPT_POSTFIELDS, data);
		if (CURLE_OK != code)
		{
			return -1;
		}
	}
	code = curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, (int)size);
	if (CURLE_OK != code)
	{
		return -1;
	}

	code = curl_easy_perform(conn);
	if (CURLE_OK != code)
	{
		return -1;
	}

	curl_easy_cleanup(conn);

	return 1;
}
