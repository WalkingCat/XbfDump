#include "stdafx.h"
#include "xbf_parser.h"

using namespace std;

xbf_parser::xbf_parser(easy_file&& file) : m_file(std::move(file)) {}

bool xbf_parser::read_string(std::wstring* s)
{
	uint32_t length = 0;

	bool ret = m_file.read(&length);

	if (ret) {
		if (s) {
			s->resize(length);
			ret = m_file.read(const_cast<wchar_t*>(s->c_str()), length * sizeof(wchar_t));
		} else {
			ret = m_file.skip(length * sizeof(wchar_t));
		}
	}
	
	return ret;
}

bool xbf_parser::read_node(xbf_node& node)
{
	bool ret = m_file.read(&node.node_id);
	if (ret) ret = m_file.read(&node.node_flags);
	return ret;
}

xbf_parser::parse_result xbf_parser::parse(std::string& error)
{
	parse_result ret = result_none;

	bool status = false;

	if (status = m_file.read(&m_data.header, sizeof(xbf_header))) {
		if (m_data.header.magicNumber == 'FBX') {
			ret = (parse_result)(ret | result_header);
		} else {
			status = false;
			error = "magic number does not match";
		}
	}

	const bool is_xbf_2_1 = (m_data.header.majorFileVersion == 2) && (m_data.header.minorFileVersion == 1);

	uint32_t count = 0;
	
	if (status) status = m_file.read(&count);
	if (status) {
		m_data.strings.resize(count);
		for (uint32_t i = 0; i < count; i++) {
			if (!(status = read_string(&m_data.strings[i]))) break;
			if (is_xbf_2_1) m_file.skip(2); // trailing zero ?
		}
	}

	if (status) status = m_file.read(&count);
	if (status) {
		m_data.assemblies.resize(count);
		status = m_file.read(m_data.assemblies.data(), count * sizeof(xbf_assembly));
	}

	if (status) status = m_file.read(&count);
	if (status) {
		m_data.type_namespaces.resize(count);
		status = m_file.read(m_data.type_namespaces.data(), count * sizeof(xbf_type_namespace));
	}

	if (status) status = m_file.read(&count);
	if (status) {
		m_data.types.resize(count);
		status = m_file.read(m_data.types.data(), count * sizeof(xbf_type));
	}

	if (status) status = m_file.read(&count);
	if (status) {
		m_data.properties.resize(count);
		status = m_file.read(m_data.properties.data(), count * sizeof(xbf_property));
	}

	if (status) status = m_file.read(&count);
	if (status) {
		m_data.xml_namespaces.resize(count);
		status = m_file.read(m_data.xml_namespaces.data(), count * sizeof(xbf_xml_namespace));
	}

	if (status) ret = (parse_result)(ret | result_metadata);
	
	if (status) {
		if (m_data.header.majorFileVersion == 1) {
			status = read_nodes_v1();
			if (status)
				ret = (parse_result)(ret | result_nodes);
			else error = "node stream parsing failed";
		} else {
			error = string("file version not supported: ") + to_string(m_data.header.majorFileVersion) + "." + to_string(m_data.header.minorFileVersion);
		}
	}

	return ret;
}
