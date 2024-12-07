#include "gizmo_icon_pool.h"

#include <vector>
#include <unordered_map>
#include <stb_image.h>

#include "../core/utils/iohandler.h"
#include "../core/utils/log.h"

namespace GizmoIconPool {

	std::vector<std::string> _validExtensions = { ".png", ".jpg", ".jpeg" };

	std::unordered_map<std::string, Texture> icons;
	Texture invalidIcon = Texture::empty();

	void loadAll(std::string directoryPath)
	{
		Log::printProcessStart("Gizmo Icon Pool", "Loading icons from " + directoryPath);

		unsigned int nLoaded = 0;
		std::vector<std::string> files = IOHandler::getFilesWithExtensions(directoryPath, _validExtensions);
		for (const auto& file : files) {
			std::string filename = IOHandler::getFilenameRaw(file);
			icons.insert({ filename, Texture::load(file, TextureType::ALBEDO) });
			nLoaded++;
		}

		Log::printProcessDone("Gizmo Icon Pool", "Done: " + std::to_string(nLoaded) + (nLoaded == 1 ? " icon loaded" : " icons loaded"));
	}

	Texture& get(std::string identifier)
	{
		auto it = icons.find(identifier);
		if (it != icons.end()) {
			return it->second;
		}
		else {
			return invalidIcon;
		}
	}

}