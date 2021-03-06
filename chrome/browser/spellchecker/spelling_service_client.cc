// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/spellchecker/spelling_service_client.h"

#include "base/command_line.h"
#include "base/json/json_reader.h"
#include "base/json/string_escape.h"
#include "base/logging.h"
#include "base/prefs/pref_service.h"
#include "base/stl_util.h"
#include "base/string_util.h"
#include "base/stringprintf.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/spellcheck_common.h"
#include "chrome/common/spellcheck_result.h"
#include "google_apis/google_api_keys.h"
#include "net/base/load_flags.h"
#include "net/url_request/url_fetcher.h"

namespace {

// The URL for requesting spell checking and sending user feedback.
const char kSpellingServiceURL[] = "https://www.googleapis.com/rpc";

}  // namespace

SpellingServiceClient::SpellingServiceClient() {
}

SpellingServiceClient::~SpellingServiceClient() {
  STLDeleteContainerPairPointers(spellcheck_fetchers_.begin(),
                                 spellcheck_fetchers_.end());
}

bool SpellingServiceClient::RequestTextCheck(
    Profile* profile,
    ServiceType type,
    const string16& text,
    const TextCheckCompleteCallback& callback) {
  DCHECK(type == SUGGEST || type == SPELLCHECK);
  if (!profile || !IsAvailable(profile, type)) {
    callback.Run(false, text, std::vector<SpellCheckResult>());
    return false;
  }

  std::string language_code;
  std::string country_code;
  chrome::spellcheck_common::GetISOLanguageCountryCodeFromLocale(
      profile->GetPrefs()->GetString(prefs::kSpellCheckDictionary),
      &language_code,
      &country_code);

  // Format the JSON request to be sent to the Spelling service.
  std::string encoded_text;
  base::JsonDoubleQuote(text, false, &encoded_text);

  static const char kSpellingRequest[] =
      "{"
      "\"method\":\"spelling.check\","
      "\"apiVersion\":\"v%d\","
      "\"params\":{"
      "\"text\":\"%s\","
      "\"language\":\"%s\","
      "\"originCountry\":\"%s\","
      "\"key\":%s"
      "}"
      "}";
  std::string api_key = base::GetDoubleQuotedJson(google_apis::GetAPIKey());
  std::string request = base::StringPrintf(
      kSpellingRequest,
      type,
      encoded_text.c_str(),
      language_code.c_str(),
      country_code.c_str(),
      api_key.c_str());

  GURL url = GURL(kSpellingServiceURL);
  net::URLFetcher* fetcher = CreateURLFetcher(url);
  fetcher->SetRequestContext(profile->GetRequestContext());
  fetcher->SetUploadData("application/json", request);
  fetcher->SetLoadFlags(
      net::LOAD_DO_NOT_SEND_COOKIES | net::LOAD_DO_NOT_SAVE_COOKIES);
  spellcheck_fetchers_[fetcher] = new TextCheckCallbackData(callback, text);
  fetcher->Start();
  return true;
}

bool SpellingServiceClient::IsAvailable(Profile* profile, ServiceType type) {
  const PrefService* pref = profile->GetPrefs();
  // If prefs don't allow spellchecking or if the profile is off the record,
  // the spelling service should be unavailable.
  if (!pref->GetBoolean(prefs::kEnableContinuousSpellcheck) ||
      !pref->GetBoolean(prefs::kSpellCheckUseSpellingService) ||
      profile->IsOffTheRecord())
    return false;

  // If the locale for spelling has not been set, the user has not decided to
  // use spellcheck so we don't do anything remote (suggest or spelling).
  std::string locale = pref->GetString(prefs::kSpellCheckDictionary);
  if (locale.empty())
    return false;

  // If we do not have the spelling service enabled, then we are only available
  // for SUGGEST.
  const CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kUseSpellingSuggestions))
    return type == SUGGEST;

  // Finally, if all options are available, we only enable only SUGGEST
  // if SPELLCHECK is not available for our language because SPELLCHECK results
  // are a superset of SUGGEST results.
  // TODO(rlp): Only available for English right now. Fix this line to include
  // all languages SPELLCHECK covers.
  bool language_available = !locale.compare(0, 2, "en");
  if (language_available) {
    return type == SPELLCHECK;
  } else {
    // Only SUGGEST is allowed.
    return type == SUGGEST;
  }
}

SpellingServiceClient::TextCheckCallbackData::TextCheckCallbackData(
    TextCheckCompleteCallback callback,
    string16 text)
      : callback(callback),
        text(text) {
}

SpellingServiceClient::TextCheckCallbackData::~TextCheckCallbackData() {
}

void SpellingServiceClient::OnURLFetchComplete(
    const net::URLFetcher* source) {
  DCHECK(spellcheck_fetchers_[source]);
  scoped_ptr<const net::URLFetcher> fetcher(source);
  scoped_ptr<TextCheckCallbackData>
      callback_data(spellcheck_fetchers_[fetcher.get()]);
  bool success = false;
  std::vector<SpellCheckResult> results;
  if (fetcher->GetResponseCode() / 100 == 2) {
    std::string data;
    fetcher->GetResponseAsString(&data);
    success = ParseResponse(data, &results);
  }
  callback_data->callback.Run(success, callback_data->text, results);
  spellcheck_fetchers_.erase(fetcher.get());
}

net::URLFetcher* SpellingServiceClient::CreateURLFetcher(const GURL& url) {
  return net::URLFetcher::Create(url, net::URLFetcher::POST, this);
}

bool SpellingServiceClient::ParseResponse(
    const std::string& data,
    std::vector<SpellCheckResult>* results) {
  // When this JSON-RPC call finishes successfully, the Spelling service returns
  // an JSON object listed below.
  // * result - an envelope object representing the result from the APIARY
  //   server, which is the JSON-API front-end for the Spelling service. This
  //   object consists of the following variable:
  //   - spellingCheckResponse (SpellingCheckResponse).
  // * SpellingCheckResponse - an object representing the result from the
  //   Spelling service. This object consists of the following variable:
  //   - misspellings (optional array of Misspelling)
  // * Misspelling - an object representing a misspelling region and its
  //   suggestions. This object consists of the following variables:
  //   - charStart (number) - the beginning of the misspelled region;
  //   - charLength (number) - the length of the misspelled region;
  //   - suggestions (array of string) - the suggestions for the misspelling
  //     text, and;
  //   - canAutoCorrect (optional boolean) - whether we can use the first
  //     suggestion for auto-correction.
  // For example, the Spelling service returns the following JSON when we send a
  // spelling request for "duck goes quisk" as of 16 August, 2011.
  // {
  //   "result": {
  //     "spellingCheckResponse": {
  //       "misspellings": [{
  //           "charStart": 10,
  //           "charLength": 5,
  //           "suggestions": [{ "suggestion": "quack" }],
  //           "canAutoCorrect": false
  //       }]
  //     }
  //   }
  // }
  scoped_ptr<DictionaryValue> value(
      static_cast<DictionaryValue*>(
          base::JSONReader::Read(data, base::JSON_ALLOW_TRAILING_COMMAS)));
  if (!value.get() || !value->IsType(base::Value::TYPE_DICTIONARY))
    return false;

  // Retrieve the array of Misspelling objects. When the input text does not
  // have misspelled words, it returns an empty JSON. (In this case, its HTTP
  // status is 200.) We just return true for this case.
  ListValue* misspellings = NULL;
  const char kMisspellings[] = "result.spellingCheckResponse.misspellings";
  if (!value->GetList(kMisspellings, &misspellings))
    return true;

  for (size_t i = 0; i < misspellings->GetSize(); ++i) {
    // Retrieve the i-th misspelling region and put it to the given vector. When
    // the Spelling service sends two or more suggestions, we read only the
    // first one because SpellCheckResult can store only one suggestion.
    DictionaryValue* misspelling = NULL;
    if (!misspellings->GetDictionary(i, &misspelling))
      return false;

    int start = 0;
    int length = 0;
    ListValue* suggestions = NULL;
    if (!misspelling->GetInteger("charStart", &start) ||
        !misspelling->GetInteger("charLength", &length) ||
        !misspelling->GetList("suggestions", &suggestions)) {
      return false;
    }

    DictionaryValue* suggestion = NULL;
    string16 replacement;
    if (!suggestions->GetDictionary(0, &suggestion) ||
        !suggestion->GetString("suggestion", &replacement)) {
      return false;
    }
    SpellCheckResult result(
        SpellCheckResult::SPELLING, start, length, replacement);
    results->push_back(result);
  }
  return true;
}
