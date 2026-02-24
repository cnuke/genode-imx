/*
 * \brief  Driver for Freescale's i.MX UART + RX/IRQ
 * \author Sebastian Sumpf
 * \date   2026-03-03
 */

/*
 * Copyright (C) 2012-2026 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _IMX_UART_H_
#define _IMX_UART_H_

/* Genode includes */
#include <platform_session/device.h>
#include <hw/spec/arm/imx_uart.h>

namespace Genode {
	class Imx_uart;
	using Mmio_device = Platform::Device::Mmio<0xbc>;
	using Uart        = Hw::Imx_uart;
}


class Genode::Imx_uart : Mmio_device,
                         public Hw::Imx_uart
{
	private:

		void _init()
		{
			Mmio_device::write<Uart::Cr3::Rxdmux_sel>(1);
			Mmio_device::write<Uart::Cr2::Rx_en>(1);
		}

	public:

		Imx_uart(Platform::Device &device)
		:
		  Mmio_device(device),
		  Uart(reinterpret_cast<addr_t>(local_addr<addr_t>()), 0, 0)
		{
			_init();
		}

		void enable_irq()
		{
			Mmio_device::write<Uart::Cr4::Dr_en>(1);
		}


		bool char_avail()
		{
			return Mmio_device::read<Uart::Sr2::Rdr>() == 1;
		}

		char get_char()
		{
			return char(Mmio_device::read<Uart::Rxd::Rx_data>());
		}
};

#endif /* _IMX_UART_H_ */
