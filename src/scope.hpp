#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <list>
#include <utility>

enum class ContentType {
    Unknown,
    URL,
    FilePath,
    Directory,
    GrepOutput,      // file:line:text format
    VimgrepOutput,   // file:line:col:text format
};

struct ParsedContent {
    ContentType type = ContentType::Unknown;
    std::string filepath;
    std::string url;
    int line = 0;
    int col = 0;
    std::string rawContent;
};

class Scope {
public:
    Scope() = default;

    // Main entry point: process input and return preview content
    std::string Process(const std::string& input);

    // Detect content type from input string
    ParsedContent Parse(const std::string& input) const;

    // Clear the preview cache
    void ClearCache();

private:
    // Rendering methods
    std::string RenderURL(const std::string& url) const;
    std::string RenderFile(const std::string& filepath) const;
    std::string RenderDirectory(const std::string& dirpath) const;
    std::string RenderFileAtLine(const std::string& filepath, int line) const;
    std::string RenderUnknown(const std::string& content) const;

    // LRU Cache (key: input string, value: rendered content)
    static constexpr size_t MAX_CACHE_SIZE = 50;
    std::list<std::pair<std::string, std::string>> m_cacheList;
    std::unordered_map<std::string, decltype(m_cacheList)::iterator> m_cacheMap;

    void CacheInsert(const std::string& key, const std::string& value);
    std::string* CacheLookup(const std::string& key);
};
