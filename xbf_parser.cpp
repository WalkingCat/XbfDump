#include "stdafx.h"
#include "xbf_parser.h"

xbf_parser::xbf_parser(easy_file&& file) : m_file(std::move(file)) {}

bool xbf_parser::read_nodes()
{
	bool ret = true;

	while (ret && (!m_file.end())) {
		xbf_node_type obj_type = xntNone;
		if (m_file.read(&obj_type)) {
			switch (obj_type) {
				case xntLineInfo: {
					auto node = std::make_unique<xbf_node_line_info>();
					ret = m_file.read(&node->line_num_delta);
					if (ret) ret = m_file.read(&node->line_pos_delta);
					if (ret) m_data.nodes.push_back(std::move(node));
					break;
				}
				case xntLineInfoAbsolute: {
					auto node = std::make_unique<xbf_node_line_info_abs>();
					ret = m_file.read(&node->line_num);
					if (ret) ret = m_file.read(&node->line_pos);
					if (ret) m_data.nodes.push_back(std::move(node));
					break;
				}
				case xntNamespace: {
					auto node = std::make_unique<xbf_node_namespace>();
					ret = read_node(*node);
					if (ret) ret = read_string(&node->name);
					if (ret) m_data.nodes.push_back(std::move(node));
					break;
				}
				case xntNone:
					m_data.nodes.push_back(std::make_unique<xbf_node_data>(xntNone));
					break; // what ?
				case xntStartObject: {
					auto node = std::make_unique<xbf_node>(xntStartObject);
					ret = read_node(*node);
					if (ret) m_data.nodes.push_back(std::move(node));
					break;
				}
				case xntEndObject:
					m_data.nodes.push_back(std::make_unique<xbf_node_data>(xntEndObject));
					break;
				case xntStartProperty: {
					auto node = std::make_unique<xbf_node>(xntStartProperty);
					ret = read_node(*node);
					if (ret) m_data.nodes.push_back(std::move(node));
					break;
				}
				case xntEndProperty:
					m_data.nodes.push_back(std::make_unique<xbf_node_data>(xntEndProperty));
					break;
				case xntText: {
					auto node = std::make_unique<xbf_node>(xntText);
					ret = read_node(*node);
					if (ret) m_data.nodes.push_back(std::move(node));
					break;
				}
				case xntValue: {
					xbf_node_value_type value_type = xnvtNone;
					if (ret = m_file.read(&value_type)) {
						switch (value_type) {
							case xnvtBoolFalse:
							case xnvtBoolTrue:
								m_data.nodes.push_back(std::make_unique<xbf_node_value>(value_type));
								break;
							case xnvtFloat:
							case xnvtKeyTime:
							case xnvtLengthConverter:
							case xnvtDuration: {
								auto node = std::make_unique<xbf_node_value_data<float>>(value_type);
								ret = m_file.read(&node->data);
								if (ret) m_data.nodes.push_back(std::move(node));
								break;
							}
							case xnvtSigned: {
								auto node = std::make_unique<xbf_node_value_data<int32_t>>(value_type);
								ret = m_file.read(&node->data);
								if (ret) m_data.nodes.push_back(std::move(node));
								break;
							}
							case xnvtCString:{
								auto node = std::make_unique<xbf_node_value_data<std::wstring>>(value_type);
								ret = read_string(&node->data);
								if (ret) m_data.nodes.push_back(std::move(node));
								break;
							}
							case xnvtThickness: {
								auto node = std::make_unique<xbf_node_value_data<std::tuple<float, float, float, float>>>(value_type);
								float v1, v2, v3, v4;
								ret = m_file.read(&v1);
								if (ret) ret = m_file.read(&v2);
								if (ret) ret = m_file.read(&v3);
								if (ret) ret = m_file.read(&v4);
								if (ret) {
									node->data = std::make_tuple(v1, v2, v3, v4);
									m_data.nodes.push_back(std::move(node));
								}
								break;
							}
							case xnvtGridLength: {
								auto node = std::make_unique<xbf_node_value_data<std::tuple<uint32_t, float>>>(value_type);
								uint32_t v1; float v2;
								ret = m_file.read(&v1);
								if (ret) ret = m_file.read(&v2);
								if (ret) {
									node->data = std::make_tuple(v1, v2);
									m_data.nodes.push_back(std::move(node));
								}
								break;
							}
							case xnvtColor: {
								auto node = std::make_unique<xbf_node_value_data<uint32_t>>(value_type);
								ret = m_file.read(&node->data);
								if (ret) m_data.nodes.push_back(std::move(node));
								break;
							}
							default:
								ret = false;
								break;
						}
					}
					break;
				}
				case xntEndOfAttributes:
					m_data.nodes.push_back(std::make_unique<xbf_node_data>(xntEndOfAttributes));
					break;
				case xntEndOfStream:
					m_data.nodes.push_back(std::make_unique<xbf_node_data>(xntEndOfStream));
					break;
				default:
					ret = false;
					break;
			}
		}
	}

	return ret;
}

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

bool xbf_parser::parse()
{
	bool ret = true;

	if (ret = m_file.read(&m_data.header, sizeof(xbf_header))) {
		if (m_data.header.magicNumber != 'FBX') ret = false;
	}

	uint32_t count = 0;
	
	if (ret) ret = m_file.read(&count);
	if (ret) {
		m_data.strings.resize(count);
		for (uint32_t i = 0; i < count; i++) {
			if(!(ret = read_string(&m_data.strings[i]))) break;
		}
	}

	if (ret) ret = m_file.read(&count);
	if (ret) {
		m_data.assemblies.resize(count);
		ret = m_file.read(m_data.assemblies.data(), count * sizeof(xbf_assembly));
	}

	if (ret) ret = m_file.read(&count);
	if (ret) {
		m_data.type_namespaces.resize(count);
		ret = m_file.read(m_data.type_namespaces.data(), count * sizeof(xbf_type_namespace));
	}

	if (ret) ret = m_file.read(&count);
	if (ret) {
		m_data.types.resize(count);
		ret = m_file.read(m_data.types.data(), count * sizeof(xbf_type));
	}

	if (ret) ret = m_file.read(&count);
	if (ret) {
		m_data.properties.resize(count);
		ret = m_file.read(m_data.properties.data(), count * sizeof(xbf_property));
	}

	if (ret) ret = m_file.read(&count);
	if (ret) {
		m_data.xml_namespaces.resize(count);
		ret = m_file.read(m_data.xml_namespaces.data(), count * sizeof(xbf_xml_namespace));
	}
	
	if (ret) ret = read_nodes();

	return ret;
}
