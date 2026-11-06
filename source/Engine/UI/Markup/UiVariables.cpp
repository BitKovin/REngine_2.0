#include "UiVariables.h"

#include <Localization/Localisation.h>
#include <UI/Elements/UiElem.h>
#include <UI/Markup/UiMarkupLog.h>

#include <cstdio>
#include <cstdlib>

namespace
{
    constexpr char kOpenEscape = '\x01';
    constexpr char kCloseEscape = '\x02';

    bool Lookup(const UiElem& element, const std::string& name, std::string& value, double& number, bool& hasNumber)
    {
        for (const UiElem* e = &element; e != nullptr; e = e->AsParentElem())
        {
            const auto it = e->m_vars.find(name);
            if (it == e->m_vars.end())
                continue;
            value = it->second;
            const auto num = e->m_numVars.find(name);
            if (num != e->m_numVars.end())
            {
                number = num->second;
                hasNumber = true;
            }
            return true;
        }
        return false;
    }

    bool FormatNumber(const std::string& fmt, double number, std::string& out)
    {
        if (fmt.size() < 2 || fmt[0] != '%')
            return false;
        const char type = fmt.back();
        if (type != 'f' && type != 'd' && type != 'i')
            return false;
        for (size_t i = 1; i + 1 < fmt.size(); ++i)
        {
            const char c = fmt[i];
            if (!((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == ' ' || c == '0'))
                return false;
        }
        char buffer[128];
        if (type == 'f')
            std::snprintf(buffer, sizeof(buffer), fmt.c_str(), number);
        else
        {
            std::string longFmt = fmt.substr(0, fmt.size() - 1) + "lld";
            std::snprintf(buffer, sizeof(buffer), longFmt.c_str(), (long long)number);
        }
        out = buffer;
        return true;
    }

    std::string Expand(const UiElem& element, const std::string& text, std::vector<std::string>* used, int depth)
    {
        std::string out;
        const size_t n = text.size();
        for (size_t i = 0; i < n; ++i)
        {
            const char c = text[i];
            if (c == '{' && i + 1 < n && text[i + 1] == '{') { out += kOpenEscape; ++i; continue; }
            if (c == '}' && i + 1 < n && text[i + 1] == '}') { out += kCloseEscape; ++i; continue; }
            if (c == '{' && i > 0 && text[i - 1] == '$') { out += c; continue; }   // ${KEY}
            if (c != '{') { out += c; continue; }

            const size_t close = text.find('}', i + 1);
            if (close == std::string::npos) { out += c; continue; }
            std::string body = text.substr(i + 1, close - i - 1);
            std::string name = body, fmt;
            const size_t colon = body.find(':');
            if (colon != std::string::npos)
            {
                name = body.substr(0, colon);
                fmt = body.substr(colon + 1);
            }
            if (name.empty()) { out += c; continue; }
            if (used != nullptr)
                used->push_back(name);

            std::string value;
            double number = 0.0;
            bool hasNumber = false;
            if (!Lookup(element, name, value, number, hasNumber))
            {
                auto& warned = const_cast<UiElem&>(element).m_warnedMissingVars;
                if (!warned[name])
                {
                    warned[name] = true;
                    UiMarkupLog::Warning(element.ScopeFile, 0, 0, "variable '" + name + "' is not set");
                }
                i = close;
                continue;
            }
            if (!fmt.empty())
            {
                if (!hasNumber)
                {
                    char* end = nullptr;
                    number = std::strtod(value.c_str(), &end);
                    hasNumber = end != value.c_str() && end != nullptr && *end == '\0';
                }
                std::string formatted;
                if (hasNumber && FormatNumber(fmt, number, formatted))
                    value = formatted;
                else
                    UiMarkupLog::Warning(element.ScopeFile, 0, 0, "cannot apply format '" + fmt + "' to variable '" + name + "'");
            }
            if (depth < 4 && value.find('{') != std::string::npos)
                value = Expand(element, value, used, depth + 1);
            out += value;
            i = close;
        }
        return out;
    }
}

std::string UiSubstitute(const UiElem& element, const std::string& raw, std::vector<std::string>* usedNames)
{
    if (raw.find('{') == std::string::npos && raw.find('}') == std::string::npos && raw.find("${") == std::string::npos)
        return raw;
    std::string text = Expand(element, raw, usedNames, 0);
    if (text.find("${") != std::string::npos)
    {
        text = Localisation::LocalizeString(text);
        text = Expand(element, text, usedNames, 0);
    }
    for (char& c : text)
    {
        if (c == kOpenEscape) c = '{';
        else if (c == kCloseEscape) c = '}';
    }
    return text;
}
