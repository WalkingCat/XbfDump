#pragma once
#include "easy_file.h"
#include "xbf_data.h"

class xbf_parser
{
private:
	easy_file m_file;
	xbf_data m_data;

	bool read_node(xbf_node& node);
	bool read_string(std::wstring* s);
public:
	xbf_parser(easy_file&& file);

	bool parse();
	bool read_nodes();

	const xbf_data& get_data() { return m_data; }
};

