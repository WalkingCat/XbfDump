#include "stdafx.h"
#include "xbf_parser.h"

bool xbf_parser::read_nodes_v1()
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
