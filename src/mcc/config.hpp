#pragma once

#include <map>
#include <string>

#include <mcc/result.hpp>

namespace mcc {
	/*
		The configuration file is parsed as a collection of lines, where each non blank line is either a comment (first non white-space character is ';')
		or a key=value pair.
		The command-line arguments are parsed as follows: `game.exe [-c CONFIG_FILE_PATH] [key=value ...]`.
		The key-value pairs specified on the command-line arguments override the ones defined in the configuration file.
	*/
	class Config final {
	public:
		class Variable final {
		public:
			~Variable() = default;

			/*
				Gets the variable value as an integer.
			*/
			Result<long long, std::string> as_integer() const;

			/*
				Gets the variable value as a floating point number.
			*/
			Result<double, std::string> as_double() const;

			/*
				Gets the variable value as a string.
			*/
			const std::string& as_string() const;

		private:
			friend Config;

			Variable(const std::string& value);

			std::string value;
		};

		Config(int argc, char** argv);
		~Config() = default;

		Config(const Config&) = delete;
		Config(Config&&) = delete;
		Config& operator=(const Config&) = delete;

		Result<const Variable&, std::string> operator[](const std::string& name) const;

		void set(const std::string& key, const std::string& value);

	private:
		std::map<std::string, Variable> variables;
	};
}