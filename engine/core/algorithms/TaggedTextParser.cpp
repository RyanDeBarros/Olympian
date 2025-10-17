#include "TaggedTextParser.h"

#include "core/util/LoggerOperators.h"

#include <stack>

namespace oly::algo
{
    static std::vector<utf::String> tags_vector(const std::stack<utf::String>& tag_stack)
    {
        std::vector<utf::String> tags(tag_stack.size());
        std::stack<utf::String> temp = tag_stack;
        for (int i = tag_stack.size() - 1; i >= 0; --i)
        {
            tags[i] = temp.top();
            temp.pop();
        }
        return tags;
    }

	UTFTaggedTextParser::UTFTaggedTextParser(const utf::String& input)
	{
        std::stack<utf::String> tag_stack;
        utf::String buffer;
        
        auto it = input.begin();

        while (it)
        {
            utf::Codepoint c = it.advance();

            if (c == '\\' && it && (it.codepoint() == '<' || it.codepoint() == '>'))
            {
                // Handle escaped '<' or '>'
                buffer.push_back(it.advance());
            }
            else if (c == '<')
            {
                if (!buffer.empty())
                {
                    // Flush buffer
                    groups.push_back({ buffer, tags_vector(tag_stack) });
                    buffer.clear();
                }

                // Check if it's a closing tag
                bool closing = false;
                if (it && it.codepoint() == '/')
                {
                    closing = true;
                    it.advance();
                }

                utf::String tag;
                while (it && it.codepoint() != '>')
                    tag.push_back(it.advance());

                if (!tag.empty())
                {
                    if (closing)
                    {
                        if (!tag_stack.empty() && tag_stack.top() == tag)
                            tag_stack.pop();
                        else
                        {
                            OLY_LOG_WARNING(true, "ALGO") << LOG.source_info.full_source() << "Unmatched closing tag </" << tag << ">" << LOG.nl;
                        }
                    }
                    else
                        tag_stack.push(tag);
                }
                
                if (it && it.codepoint() == '>')
                    it.advance();
            }
            else
                buffer.push_back(c);
        }

        // Flush remaining text
        if (!buffer.empty())
            groups.push_back({ buffer, tags_vector(tag_stack) });
	}
}
