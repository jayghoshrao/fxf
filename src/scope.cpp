#include "scope.hpp"
#include "utils.hpp"

#include <regex>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

// Regex patterns for content detection
static const std::regex vimgrep_regex(R"(^(.+?):(\d+):(\d+):.*$)");
static const std::regex grep_regex(R"(^(.+?):(\d+):.*$)");

std::string Scope::Process(const std::string& input)
{
    // Check cache first
    if (auto* cached = CacheLookup(input)) {
        return *cached;
    }

    auto parsed = Parse(input);
    std::string result;

    switch (parsed.type) {
        case ContentType::VimgrepOutput:
        case ContentType::GrepOutput:
            result = RenderFileAtLine(parsed.filepath, parsed.line);
            break;
        case ContentType::URL:
            result = RenderURL(parsed.url);
            break;
        case ContentType::FilePath:
            result = RenderFile(parsed.filepath);
            break;
        case ContentType::Directory:
            result = RenderDirectory(parsed.filepath);
            break;
        case ContentType::Unknown:
        default:
            result = RenderUnknown(parsed.rawContent);
            break;
    }

    CacheInsert(input, result);
    return result;
}

ParsedContent Scope::Parse(const std::string& input) const
{
    ParsedContent result;
    result.rawContent = input;

    std::string trimmed = trim(input);
    if (trimmed.empty()) {
        result.type = ContentType::Unknown;
        return result;
    }

    // 1. Try vimgrep pattern first (most specific: file:line:col:text)
    std::smatch match;
    if (std::regex_match(trimmed, match, vimgrep_regex)) {
        std::string filepath = match[1].str();
        if (fs::exists(filepath) && fs::is_regular_file(filepath)) {
            result.type = ContentType::VimgrepOutput;
            result.filepath = filepath;
            result.line = std::stoi(match[2].str());
            result.col = std::stoi(match[3].str());
            return result;
        }
    }

    // 2. Try grep pattern (file:line:text)
    if (std::regex_match(trimmed, match, grep_regex)) {
        std::string filepath = match[1].str();
        if (fs::exists(filepath) && fs::is_regular_file(filepath)) {
            result.type = ContentType::GrepOutput;
            result.filepath = filepath;
            result.line = std::stoi(match[2].str());
            return result;
        }
    }

    // 3. Try URL extraction
    std::string url = ExtractFirstURL(trimmed);
    if (!url.empty()) {
        result.type = ContentType::URL;
        result.url = url;
        return result;
    }

    // 4. Try file path or directory
    if (fs::exists(trimmed)) {
        if (fs::is_regular_file(trimmed)) {
            result.type = ContentType::FilePath;
            result.filepath = trimmed;
            return result;
        }
        if (fs::is_directory(trimmed)) {
            result.type = ContentType::Directory;
            result.filepath = trimmed;
            return result;
        }
    }

    result.type = ContentType::Unknown;
    return result;
}

std::string Scope::RenderURL(const std::string& url) const
{
    // Escape single quotes for shell safety
    std::string escaped;
    for (char c : url) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped += c;
        }
    }

    // Use timeout to prevent hangs (5 second timeout)
    const std::string timeout = "timeout 5 ";

    // Try w3m first, fall back to lynx, then curl
    std::string cmd = timeout + "w3m -dump '" + escaped + "' 2>/dev/null | head -n 50";

    try {
        std::string result = ExecAndCapture(cmd);
        if (!result.empty()) {
            return result;
        }
    } catch (...) {
        // w3m failed, try fallback
    }

    // Fallback to lynx
    cmd = timeout + "lynx -dump -nolist '" + escaped + "' 2>/dev/null | head -n 50";
    try {
        std::string result = ExecAndCapture(cmd);
        if (!result.empty()) {
            return result;
        }
    } catch (...) {
        // lynx failed, try curl
    }

    // Last resort: curl with timeout flag
    cmd = "curl -sL --max-time 5 '" + escaped + "' 2>/dev/null | head -n 50";
    try {
        std::string result = ExecAndCapture(cmd);
        if (!result.empty()) {
            return result;
        }
    } catch (...) {
        // curl also failed
    }

    return "[Timeout or failed to fetch URL: " + url + "]";
}

std::string Scope::RenderFile(const std::string& filepath) const
{
    // Escape single quotes for shell safety
    std::string escaped;
    for (char c : filepath) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped += c;
        }
    }

    // Check if it's a binary file
    std::string fileCmd = "file -b '" + escaped + "' 2>/dev/null";
    try {
        std::string fileType = ExecAndCapture(fileCmd);
        if (fileType.find("text") == std::string::npos &&
            fileType.find("ASCII") == std::string::npos &&
            fileType.find("UTF") == std::string::npos &&
            fileType.find("empty") == std::string::npos) {
            return "[Binary file: " + filepath + "]\n" + fileType;
        }
    } catch (...) {
        // Ignore file command failure, try to read anyway
    }

    std::string cmd = "head -n 50 '" + escaped + "' 2>/dev/null";
    try {
        return ExecAndCapture(cmd);
    } catch (...) {
        return "[Failed to read file: " + filepath + "]";
    }
}

std::string Scope::RenderDirectory(const std::string& dirpath) const
{
    // Escape single quotes for shell safety
    std::string escaped;
    for (char c : dirpath) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped += c;
        }
    }

    std::string cmd = "ls -la '" + escaped + "' 2>/dev/null | head -n 50";
    try {
        std::ostringstream header;
        header << "=== " << dirpath << "/ ===\n\n";
        return header.str() + ExecAndCapture(cmd);
    } catch (...) {
        return "[Failed to list directory: " + dirpath + "]";
    }
}

std::string Scope::RenderFileAtLine(const std::string& filepath, int line) const
{
    // Escape single quotes for shell safety
    std::string escaped;
    for (char c : filepath) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped += c;
        }
    }

    int contextLines = 10;
    int startLine = std::max(1, line - contextLines);
    int endLine = line + contextLines;

    std::string cmd = "sed -n '" + std::to_string(startLine) + "," +
                      std::to_string(endLine) + "p' '" + escaped + "' 2>/dev/null";

    try {
        std::string content = ExecAndCapture(cmd);

        // Add a header showing file:line
        std::ostringstream header;
        header << "=== " << filepath << ":" << line << " ===\n\n";

        return header.str() + content;
    } catch (...) {
        return "[Failed to read file: " + filepath + "]";
    }
}

std::string Scope::RenderUnknown(const std::string& content) const
{
    return content;
}

void Scope::ClearCache()
{
    m_cacheList.clear();
    m_cacheMap.clear();
}

void Scope::CacheInsert(const std::string& key, const std::string& value)
{
    // If key exists, move to front and update value
    auto it = m_cacheMap.find(key);
    if (it != m_cacheMap.end()) {
        m_cacheList.erase(it->second);
        m_cacheMap.erase(it);
    }

    // Evict oldest if at capacity
    if (m_cacheList.size() >= MAX_CACHE_SIZE) {
        auto& oldest = m_cacheList.back();
        m_cacheMap.erase(oldest.first);
        m_cacheList.pop_back();
    }

    // Insert at front
    m_cacheList.emplace_front(key, value);
    m_cacheMap[key] = m_cacheList.begin();
}

std::string* Scope::CacheLookup(const std::string& key)
{
    auto it = m_cacheMap.find(key);
    if (it == m_cacheMap.end()) {
        return nullptr;
    }

    // Move to front (LRU)
    m_cacheList.splice(m_cacheList.begin(), m_cacheList, it->second);
    return &it->second->second;
}
