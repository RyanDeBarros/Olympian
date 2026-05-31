#include "DocumentManager.h"

#include "IDocument.h"

#include "core/MainWindow.h"

DocumentManager& DocumentManager::Instance()
{
	return MainWindow::Instance().GetDocumentManager();
}

void DocumentManager::Draw()
{
	for (const auto& doc : _documents)
		doc->Draw();
}

size_t DocumentManager::DocumentCount() const
{
	return _documents.size();
}

const IDocument& DocumentManager::GetDocument(size_t i) const
{
	return *_documents[i];
}

IDocument& DocumentManager::GetDocument(size_t i)
{
	return *_documents[i];
}

void DocumentManager::Add(std::unique_ptr<IDocument>&& doc)
{
	_documents.push_back(std::move(doc));
}

void DocumentManager::Remove(IDocument& document)
{
	for (auto it = _documents.begin(); it != _documents.end(); ++it)
	{
		if (it->get() == &document)
		{
			_documents.erase(it);
			break;
		}
	}
}

void DocumentManager::Remove(size_t i)
{
	_documents.erase(_documents.begin() + i);
}
