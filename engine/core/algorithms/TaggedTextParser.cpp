#include "TaggedTextParser.h"

#include "core/util/LoggerOperators.h"

#include <stack>

namespace oly::algo
{
	UTFTaggedTextParser::UTFTaggedTextParser(const imp::utf::string& input)
	{
        std::stack<imp::utf::string> tag_stack;
        imp::utf::string buffer;
        
        auto it = input.begin();

        while (it)
        {
            imp::utf::codepoint c = it.advance();

            if (c == '\\' && it && (*it == '<' || *it == '>'))
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
                if (it && *it == '/')
                {
                    closing = true;
                    it.advance();
                }

                imp::utf::string tag;
                while (it && *it != '>')
                    tag.push_back(it.advance());

                if (!tag.empty())
                {
                    if (closing)
                    {
                        if (!tag_stack.empty() && tag_stack.top().begins_with(tag))
                            tag_stack.pop();
                        else
                            _OLY_ENGINE_LOG_WARNING("ALGO") << "Unmatched closing tag </" << tag << ">" << LOG.nl;
                    }
                    else
                        tag_stack.push(tag);
                }
                
                if (it && *it == '>')
                    it.advance();
            }
            else
                buffer.push_back(c);
        }

        // Flush remaining text
        if (!buffer.empty())
            groups.push_back({ buffer, tag_stack });
        
        if (!tag_stack.empty())
            _OLY_ENGINE_LOG_WARNING("ALGO") << "Not all tags were closed by the end of input string" << LOG.nl;
	}
}
