#include "TaggedTextParser.h"

#include "core/util/LoggerOperators.h"

#include <stack>

namespace oly::algo
{
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
                    groups.push_back({ buffer, tag_stack });
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
                        if (!tag_stack.empty() && tag_stack.top().begins_with(tag))
                            tag_stack.pop();
                        else
                            OLY_LOG_WARNING(true, "ALGO") << LOG.source_info.full_source() << "Unmatched closing tag </" << tag << ">" << LOG.nl;
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
            groups.push_back({ buffer, tag_stack });
        
        if (!tag_stack.empty())
            OLY_LOG_WARNING(true, "ALGO") << LOG.source_info.full_source() << "Not all tags were closed by the end of input string" << LOG.nl;
	}
}
