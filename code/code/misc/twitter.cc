#include "extern.h"
#include "colorstring.h"
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

  if(gamePort!=Config::Port::PROD)
    return false;

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";

  from=stripColorCodes(from).cap();
  msg=stripColorCodes(msg);

  // manpages: curl_global_init(CURL_GLOBAL_ALL) is not required for simple calls
  // of curl - curl_easy_init will end up doing the alloc for us if needed
  // this may save us a tiny amount of time on the init call

  curl = curl_easy_init();
  curl_formadd(&formpost, &lastptr, 
              CURLFORM_COPYNAME, "status",
              CURLFORM_COPYCONTENTS, 
              (format("%s: %s")% from% msg).str().c_str(),
              CURLFORM_END);

  headerlist = curl_slist_append(headerlist, buf);

  curl_easy_setopt(curl, CURLOPT_URL, 
                  "http://twitter.com/statuses/update.xml");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
  curl_easy_setopt(curl, CURLOPT_USERPWD, "sneezymud:kegenlgn");
  curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummyWriter);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1l); // at most take 1 sec

  curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl_formfree(formpost);
  curl_slist_free_all (headerlist);

  return true;
}

