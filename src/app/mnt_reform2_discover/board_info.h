/*
 * \brief  Board discovery information
 * \author Norman Feske
 * \author Stefan Kalkowski
 * \date   2018-06-01
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _BOARD_INFO_H_
#define _BOARD_INFO_H_

#include <base/node.h>

namespace Reform_discover {
	struct Board_info;
	using namespace Genode;
}

struct Reform_discover::Board_info
{
	enum class Pci_class : unsigned { NVME = 0x10802 };

	static bool _matches_class(Node const &pci, Pci_class value)
	{
		return pci.attribute_value("class", 0U) == unsigned(value);
	};

	/**
	 * Runtime-detected features
	 */
	struct Detected
	{
		bool nvme;

		void print(Output &out) const {
			Genode::print(out, "nvme=", nvme); }

		static inline Detected from_node(Node const &devices);

	} detected;
};


Reform_discover::Board_info::Detected
Reform_discover::Board_info::Detected::from_node(Node const &devices)
{
	Detected detected { };

	devices.for_each_sub_node("device", [&] (Node const &device) {

		device.with_optional_sub_node("pci-config", [&] (Node const &pci) {

			if (_matches_class(pci, Pci_class::NVME)) detected.nvme = true;
		});
	});

	return detected;
}

#endif /* _BOARD_INFO_H_ */
