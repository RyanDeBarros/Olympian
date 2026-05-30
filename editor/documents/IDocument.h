#pragma once

#include <string>

class IDocument
{
public:
	virtual ~IDocument() = default;

	virtual std::string GetTitle() const = 0;
	virtual void Draw() = 0;
};
