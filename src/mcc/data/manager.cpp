#include <mcc/data/manager.hpp>

#include <fstream>
#include <sstream>

using namespace mcc;
using namespace mcc::data;

Result<Handle<void>, std::string> Manager::get(const std::string& id) {
	auto it = this->loaders_by_asset.find(id);
	if (it == this->loaders_by_asset.end()) {
		std::stringstream ss;
		ss << "mcc::data::Manager::get() failed:" << std::endl;
		ss << "No asset '" << id << "' found";
		return Result<Handle<void>, std::string>::error(ss.str());
	}

	auto loader = it->second;
	auto index = loader->get_asset(id);
	if (index == -1) {
		std::stringstream ss;
		ss << "mcc::data::Manager::get() failed:" << std::endl;
		ss << "No asset '" << id << "' found on loader";
		return Result<Handle<void>, std::string>::error(ss.str());
	}

	return Result<Handle<void>, std::string>::success(Handle<void>(index, loader));
}

Manager::Manager(const Config& config, const std::map<std::string, Loader*>& loaders) {
	this->loaders_by_type = loaders;
    auto data_folder = config["data.folder"].unwrap().as_string();
    auto assets_cfg = data_folder + "assets.cfg";

	// Parse variables from config file
	std::ifstream fs(assets_cfg);
	if (!fs.is_open()) {
		std::cerr << "mcc::data::Manager::Manager() failed:" << std::endl;
		std::cerr << "Couldn't open configuration file on path \"" << assets_cfg << "\"" << std::endl;
		return;
	}

	std::string line;
	while (std::getline(fs, line)) {
		// Remove comments
		int i = 0;
		for (; i < line.size() && line[i] != ';'; ++i);
		line = line.substr(0, i);

		// Remove whitespace
		for (i = 0; i < line.size() && (line[i] == ' ' || line[i] == '\t'); ++i);
		line = line.substr(i);
		for (i = int(line.size()) - 1; i >= 0 && (line[i] == ' ' || line[i] == '\t'); --i);
		line = line.substr(0, size_t(i) + 1);
		if (line.empty())
			continue;

		// Search for assignment operator
		bool found = false;
		for (i = 0; i < line.size(); ++i) {
			if (line[i] == '=') {
				found = true;
				break;
			}
		}

		if (found) {
			auto key = line.substr(0, i);
			auto value = line.substr(size_t(i) + 1);

			// Remove whitespace
			for (i = 0; i < key.size() && (key[i] == ' ' || key[i] == '\t'); ++i);
			key = key.substr(i);
			for (i = int(key.size()) - 1; i >= 0 && (key[i] == ' ' || key[i] == '\t'); --i);
			key = key.substr(0, size_t(i) + 1);
			for (i = 0; i < value.size() && (value[i] == ' ' || value[i] == '\t'); ++i);
			value = value.substr(i);
			for (i = int(value.size()) - 1; i >= 0 && (value[i] == ' ' || value[i] == '\t'); --i);
			value = value.substr(0, size_t(i) + 1);

			if (key.empty()) {
				std::cerr << "mcc::data::Manager::Manager() failed:" << std::endl;
				std::cerr << "Couldn't parse configuration file:" << std::endl;
				std::cerr << "Invalid 'key=value' pair on \"" << line << "\"" << std::endl;
				std::cerr << "The key string must not be empty" << std::endl;
				continue; // Skip line
			}

			// Split value into usage mode, type and arguments
			std::string usage = value.substr(0, value.find(' '));
			value = value.substr(value.find(' ') + 1);
			std::string type = value.substr(0, value.find(' '));
			std::string arguments = value.substr(value.find(' ') + 1);

			bool dynamic;
			if (usage == "static") {
				dynamic = false;
			}
			else if (usage == "dynamic") {
				dynamic = true;
			}
			else {
				std::cerr << "mcc::data::Manager::Manager() failed:" << std::endl;
				std::cerr << "Couldn't parse configuration file:" << std::endl;
				std::cerr << "Invalid usage mode \"" << usage << "\" on \"" << line << "\"" << std::endl;
				std::cerr << "The usage mode must be either \"static\" or \"dynamic\"" << std::endl;
				continue; // Skip line
			}

			auto it = this->loaders_by_type.find(type);
			if (it == this->loaders_by_type.end()) {
				std::cerr << "mcc::data::Manager::Manager() failed:" << std::endl;
				std::cerr << "Couldn't parse configuration file:" << std::endl;
				std::cerr << "Unknown asset type \"" << type << "\" on \"" << line << "\"" << std::endl;
				std::cerr << "No loader found for this type" << std::endl;
				continue; // Skip line
			}

			auto loader = it->second;

			if (this->loaders_by_asset.find(key) != this->loaders_by_asset.end()) {
				std::cerr << "mcc::data::Manager::Manager() failed:" << std::endl;
				std::cerr << "Couldn't parse configuration file:" << std::endl;
				std::cerr << "More than one asset has the ID \"" << key << "\"" << std::endl;
				std::cerr << "Ignoring all but the first asset" << std::endl;
				continue; // Skip line
			}

			this->loaders_by_asset.insert({ key, loader });

			auto result = loader->add_entry(key, dynamic, arguments);

			if (result.is_error()) {
				std::cerr << "mcc::data::Manager::Manager() failed:" << std::endl;
				std::cerr << "Couldn't load static asset \"" << key << "\":" << std::endl;
				std::cerr << result.get_error() << std::endl;
				continue; // Skip line
			}
		}
		else {
			std::cerr << "mcc::data::Manager::Manager() failed:" << std::endl;
			std::cerr << "Couldn't parse configuration file:" << std::endl;
			std::cerr << "Invalid 'key=value' pair on \"" << line << "\"" << std::endl;
			continue;
		}
	}
}
