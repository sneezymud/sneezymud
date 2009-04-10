#include "stdsneezy.h"
#include "configuration.h"
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

size_t dummyWriter(void *, size_t size, size_t nmemb, void *){
  return size*nmemb;
}

bool twitterShout(sstring from, sstring msg)
{
  CURL *curl;
  CURLcode res;
  
  if(gamePort!=PROD_GAMEPORT)
    return false;

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";

  msg=stripColorCodes(msg);

  curl_global_init(CURL_GLOBAL_ALL);

  curl_formadd(&formpost, &lastptr, 
	       CURLFORM_COPYNAME, "status",
	       CURLFORM_COPYCONTENTS, 
	       (format("%s: %s")% from% msg).str().c_str(),
	       CURLFORM_END);
	       
  curl = curl_easy_init();
  headerlist = curl_slist_append(headerlist, buf);

  curl_easy_setopt(curl, CURLOPT_URL, 
		   "http://twitter.com/statuses/update.xml");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
  curl_easy_setopt(curl, CURLOPT_USERNAME, "sneezymud");
  curl_easy_setopt(curl, CURLOPT_PASSWORD, "kegenlgn");
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummyWriter);

  res = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl_formfree(formpost);
  curl_slist_free_all (headerlist);

  if(res){
    vlogf(LOG_PEEL, format("curl failed: %i") % res);
  }


  return true;

}

