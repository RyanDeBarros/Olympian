#pragma once

#include <memory>
#include <vector>

class IDocument;

class DocumentManager
{
	std::vector<std::unique_ptr<IDocument>> _documents;

public:
	static DocumentManager& Instance();

	void Draw();

	// TODO v7
	//void OpenAsset(const std::filesystem::path& path);

	size_t DocumentCount() const;
	const IDocument& GetDocument(size_t i) const;
	IDocument& GetDocument(size_t i);

	template<typename T, typename... Args>
	void Add(Args&&... args)
	{
		Add(std::make_unique<T>(std::forward<Args>(args)...));
	}

	void Add(std::unique_ptr<IDocument>&& doc);
	void Remove(IDocument& document);
	void Remove(size_t i);
};
