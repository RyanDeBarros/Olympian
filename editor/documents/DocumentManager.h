#pragma once

#include <memory>
#include <vector>

class IDocument;

class DocumentManager
{
	std::vector<std::unique_ptr<IDocument>> _documents;

public:
	void Draw();

	// TODO v7
	//void OpenAsset(const std::filesystem::path& path);
};
