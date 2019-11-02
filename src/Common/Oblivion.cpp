#include "Oblivion.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Logger
{
	boost::property_tree::ptree logTree;

	boost::property_tree::ptree notes, warnings, errors;

	void addLog(LogType type, const std::string& message)
	{
		switch (type)
		{
		case LogType::NOTE:
			notes.put(std::to_string(notes.size()), message);
			break;
		case LogType::WARNING:
			warnings.put(std::to_string(warnings.size()), message);
			break;
		case LogType::ERROR:
			errors.put(std::to_string(errors.size()), message);
			break;
		default:
			break;
		}
	}

	void dumpJson(std::ostream & stream)
	{
		boost::property_tree::ptree logs;
		logs.push_back(std::make_pair("Notes", notes));
		logs.push_back(std::make_pair("Warnings", warnings));
		logs.push_back(std::make_pair("Errors", errors));
		boost::property_tree::write_json(stream, logs);
	}
}
