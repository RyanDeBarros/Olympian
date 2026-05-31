#include "DocumentManager.h"

#include "IDocument.h"

void DocumentManager::Draw()
{
	for (const auto& doc : _documents)
		doc->Draw();
}
