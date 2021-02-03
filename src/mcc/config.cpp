#include <mcc/config.hpp>
#include <sstream>
#include <fstream>

using namespace mcc;
using Variable = Config::Variable;

mcc::Config::Config(int argc, char** argv) {
	std::string config_path = "./mcc.cfg"; // Defaut config file path
	
	// Parse variables from command-line arguments
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "-c") == 0) {
			if (i + 1 >= argc) {
				std::cerr << "mcc::Config::Config() failed:" << std::endl;
				std::cerr << "Command-line arguments parsing failed:" << std::endl;
				std::cerr << "Found '-c' but its argument is missing (unexpected end of string)" << std::endl;
				std::abort();
			}
			config_path = argv[++i];
		} else { // Parse key-value pair
			bool found = false;
			int j = 0;
			for (; argv[i][j] != '\0'; ++j) {
				if (argv[i][j] == '=') {
					found = true;
					++j;
					break;
				}
			}

			if (!found) {
				std::cerr << "mcc::Config::Config() failed:" << std::endl;
				std::cerr << "Command-line arguments parsing failed:" << std::endl;
				std::cerr << "Expected 'key=value' pair, found \"" << argv[i] << "\"" << std::endl;
				std::abort();
			}

			auto key = std::string(argv[i][0], j - 1);
			auto value = std::string(&argv[i][j]);

			this->variables.insert(std::make_pair(key, Variable(value)));
		}
	}

	// Parse variables from config file
	std::ifstream fs(config_path);
	if (!fs.is_open()) {
		std::cerr << "mcc::Config::Config() failed:" << std::endl;
		std::cerr << "Couldn't open configuration file on path \"" << config_path << "\"" << std::endl;
		std::abort();
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
				std::cerr << "mcc::Config::Config() failed:" << std::endl;
				std::cerr << "Couldn't parse configuration file:" << std::endl;
				std::cerr << "Invalid 'key=value' pair on \"" << line << "\"" << std::endl;
				std::cerr << "The key string must not be empty" << std::endl;
				std::abort();
			}

			if (this->variables.find(key) == this->variables.end())
				this->variables.insert(std::make_pair(key, Variable(value)));
		} else {
			std::cerr << "mcc::Config::Config() failed:" << std::endl;
			std::cerr << "Couldn't parse configuration file:" << std::endl;
			std::cerr << "Invalid 'key=value' pair on \"" << line << "\"" << std::endl;
			std::abort();
		}
	}
}

Result<const Variable&, std::string> mcc::Config::operator[](const std::string& name) const {
	auto it = this->variables.find(name);
	if (it == this->variables.end()) {
		std::stringstream ss;
		ss << "mcc::Config::operator[]() failed:" << std::endl << "No variable with name \"" << name << "\" found";
		return Result<const Variable&, std::string>::error(ss.str());
	}
	return Result<const Variable&, std::string>::success(it->second);
}

void mcc::Config::set(const std::string& key, const std::string& value) {
	auto it = this->variables.find(key);
	if (it != this->variables.end())
		this->variables.erase(it);
	this->variables.insert(std::make_pair(key, Variable(value)));
}

mcc::Config::Variable::Variable(const std::string& value) {
	this->value = value;
}

Result<long long, std::string> mcc::Config::Variable::as_integer() const {
	try {
		return Result<long long, std::string>::success(std::stoll(this->value));
	} catch (std::exception& e) {
		std::stringstream ss;
		ss << "mcc::Config::Variable::as_integer() failed:" << std::endl << "Variable value \"" << this->value << "\" couldn't be converted to long long:" << std::endl;
		ss << "std::stoll() failed:" << std::endl << e.what();
		return Result<long long, std::string>::error(ss.str());
	}
}

Result<double, std::string> mcc::Config::Variable::as_double() const {
	try {
		return Result<double, std::string>::success(std::stod(this->value));
	} catch (std::exception& e) {
		std::stringstream ss;
		ss << "mcc::Config::Variable::as_double() failed:" << std::endl << "Variable value \"" << this->value << "\" couldn't be converted to double:" << std::endl;
		ss << "std::stod() failed:" << std::endl << e.what();
		return Result<double, std::string>::error(ss.str());
	}
}

const std::string& mcc::Config::Variable::as_string() const {
	return this->value;
}
