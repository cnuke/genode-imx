#include <base/component.h>
#include <base/log.h>
#include <platform_session/device.h>
#include <gpio_session/connection.h>
#include <timer_session/connection.h>

using namespace Genode;

struct Pcie_config_space : Platform::Device::Mmio
{
	struct Id : Register<0x0, 32>
	{
		struct Vendor : Bitfield<0,  16> {};
		struct Device : Bitfield<16, 16> {};
	};

	using Platform::Device::Mmio::Mmio;
};


struct Pcie_controller : Platform::Device::Mmio
{
	struct Status_command : Register<0x4, 32>
	{
		struct Io_space_enable         : Bitfield<0,  1> {};
		struct Memory_space_enable     : Bitfield<1,  1> {};
		struct Bus_master_enable       : Bitfield<2,  1> {};
		struct Special_cycle_enable    : Bitfield<3,  1> {};
		struct Memory_write_invalidate : Bitfield<4,  1> {};
		struct Vga_palette_snoop       : Bitfield<5,  1> {};
		struct Parity_error_response   : Bitfield<6,  1> {};
		struct Idsel                   : Bitfield<7,  1> {};
		struct Serror_enable           : Bitfield<8,  1> {};
		struct Interrupt_enable        : Bitfield<10, 1> {};
	};

	struct Class_code_rev_id : Register<0x8, 32>
	{
		struct Revision   : Bitfield<0,  8> {};
		struct Prg_iface  : Bitfield<8,  8> {};
		struct Sub_class  : Bitfield<16, 8> {};
		struct Base_class : Bitfield<24, 8> {};
	};

	struct Base_address_0 : Register<0x10, 32> {};
	struct Base_address_1 : Register<0x14, 32> {};

	struct Sec_lat_timer_bus : Register<0x18, 32>
	{
		struct Primary_bus   : Bitfield<0,  8> {};
		struct Secondary_bus : Bitfield<8,  8> {};
		struct Sub_bus       : Bitfield<16, 8> {};
	};

	struct Bridge_ctrl_irq_pin_line : Register<0x3c, 32>
	{
		struct Irq_pin : Bitfield<8, 8> {};
	};

	struct Link_capabilities : Register<0x7c, 32>
	{
		struct Max_link_speed : Bitfield<0, 4> {};
	};

	struct Link_control2_status2 : Register<0xa0, 32>
	{
		struct Link_speed : Bitfield<0, 4> {};
	};

	struct Port_link_control : Register<0x710, 32>
	{
		struct Dll_link_enable : Bitfield<5,  1> {};
		struct Fast_link_mode  : Bitfield<7,  1> {};
		struct Link_capable    : Bitfield<16, 6> {};
	};

	struct Port_debug1 : Register<0x72c, 32>
	{
		struct Link_up          : Bitfield<4,  1> {};
		struct Link_in_training : Bitfield<29, 1> {};
	};

	struct Link_width_speed_control : Register<0x80c, 32>
	{
		struct Num_of_lanes        : Bitfield<8,  5> {};
		struct Direct_speed_change : Bitfield<17, 1> {};
	};

	struct Misc_control1 : Register<0x8bc, 32>
	{
		struct Dbi_ro_wr_enable : Bitfield<0, 1> {};
	};

	struct Atu_viewport : Register<0x900, 32>
	{
		struct Index     : Bitfield<0,  4> {};
		struct Direction : Bitfield<31, 1>
		{
			enum { OUTBOUND, INBOUND };
		};
	};

	struct Atu_region_control_1 : Register<0x904, 32>
	{
		struct Type     : Bitfield<0,  5> {};
		struct Tc       : Bitfield<5,  3> {};
		struct Td       : Bitfield<8,  1> {};
		struct Attr     : Bitfield<9,  2> {};
		struct At       : Bitfield<16, 2> {};
		struct Function : Bitfield<20, 3> {};
	};

	struct Atu_region_control_2 : Register<0x908, 32>
	{
		struct Shift_mode  : Bitfield<28, 1> {};
		struct Invert_mode : Bitfield<29, 1> {};
		struct Enable      : Bitfield<31, 1> {};
	};

	struct Atu_lower_base_addr   : Register<0x90c, 32> {};
	struct Atu_upper_base_addr   : Register<0x910, 32> {};
	struct Atu_limit_addr        : Register<0x914, 32> {};
	struct Atu_lower_target_addr : Register<0x918, 32> {};
	struct Atu_upper_target_addr : Register<0x91c, 32> {};

	enum Transaction_type {
		MEMORY_REQUEST        = 0,
		IO_REQUEST            = 2,
		CONFIG_TYPE_0_REQUEST = 4,
		CONFIG_TYPE_1_REQUEST = 5
	};

	using Platform::Device::Mmio::Mmio;

	void configure_outbound_atu(unsigned idx, Transaction_type type,
	                            addr_t cpu_addr, size_t size,
	                            addr_t pci_addr)
	{
		Atu_viewport::access_t viewport = 0;
		Atu_viewport::Direction::set(viewport,Atu_viewport::Direction::OUTBOUND);
		Atu_viewport::Index::set(viewport, idx);
		write<Atu_viewport>(viewport);

		Atu_region_control_1::access_t ctrl1 = 0;
		Atu_region_control_1::Type::set(ctrl1, type);
		write<Atu_region_control_1>(ctrl1);

		Atu_region_control_2::access_t ctrl2 = 0;
		Atu_region_control_2::Enable::set(ctrl2, 1);
		write<Atu_region_control_2>(ctrl2);

		write<Atu_lower_base_addr>(cpu_addr & 0xffffffff);
		write<Atu_upper_base_addr>(cpu_addr >> 32);
		write<Atu_limit_addr>((cpu_addr + size - 1) & 0xffffffff);
		write<Atu_lower_target_addr>(pci_addr & 0xffffffff);
		write<Atu_upper_target_addr>(pci_addr >> 32);
	}

	void init()
	{
		write<Misc_control1::Dbi_ro_wr_enable>(1);
		write<Link_control2_status2::Link_speed>(2);
		write<Link_capabilities::Max_link_speed>(2);
		write<Port_link_control::Fast_link_mode>(0);
		write<Port_link_control::Dll_link_enable>(1);
		write<Port_link_control::Link_capable>(1);
		write<Link_width_speed_control::Num_of_lanes>(1);
		write<Base_address_0>(4);
		write<Base_address_1>(0);
		write<Bridge_ctrl_irq_pin_line::Irq_pin>(1);
		write<Sec_lat_timer_bus::Primary_bus>(0);
		write<Sec_lat_timer_bus::Secondary_bus>(1);
		write<Sec_lat_timer_bus::Sub_bus>(0xff);
		write<Status_command::Io_space_enable        >(1);
		write<Status_command::Memory_space_enable    >(1);
		write<Status_command::Bus_master_enable      >(1);
		write<Status_command::Special_cycle_enable   >(0);
		write<Status_command::Memory_write_invalidate>(0);
		write<Status_command::Vga_palette_snoop      >(0);
		write<Status_command::Parity_error_response  >(0);
		write<Status_command::Idsel                  >(0);
		write<Status_command::Serror_enable          >(1);
		write<Status_command::Interrupt_enable       >(0);

		/* disable all outbound windows */
		enum { ATU_WINDOWS = 4 };
		for (unsigned i = 0; i < ATU_WINDOWS; i++) {
			Atu_viewport::access_t viewport = 0;
			Atu_viewport::Direction::set(viewport,Atu_viewport::Direction::OUTBOUND);
			Atu_viewport::Index::set(viewport, i);
			write<Atu_viewport>(viewport);

			Atu_region_control_2::access_t ctrl2 = 0;
			Atu_region_control_2::Enable::set(ctrl2, 1);
			write<Atu_region_control_2>(ctrl2);
		}

		configure_outbound_atu(1, MEMORY_REQUEST,
		                       0x18000000, 0x7f00000, 0x18000000);
		configure_outbound_atu(2, IO_REQUEST,
		                       0x1ff80000, 0x10000,   0);

		write<Base_address_0>(0);

		/* we're a PCI bridge */
		write<Class_code_rev_id::Sub_class>(0x04);
		write<Class_code_rev_id::Base_class>(0x06);
		write<Link_width_speed_control::Direct_speed_change>(1);
		write<Misc_control1::Dbi_ro_wr_enable>(0);
	}

	bool link_up()
	{
		return read<Port_debug1::Link_up>() &&
		       !read<Port_debug1::Link_in_training>();
	}

	bool atu_up()
	{
		return read<Atu_region_control_2::Enable>();
	}
};


struct Main
{
	using Reset_pin = Constructible<Platform::Device>;

	Env                & env;
	Platform::Connection platform   { env           };
	Platform::Device     device     { platform      };
	Pcie_controller      controller { device, { 0 } };
	Pcie_config_space    cfg_space  { device, { 1 } };
	Timer::Connection    timer      { env           };
	Gpio::Connection     gpio       { env, 7        };
	Reset_pin            phy_reset  {};
	Reset_pin            core_reset {};

	struct Atu_is_not_ready  {};
	struct Link_is_not_ready {};

	bool wait_for_link()
	{
		for (unsigned i = 0; i < 10; i++) {
			if (controller.link_up())
				return true;
			timer.msleep(100);
		}
		return false;
	}

	bool wait_for_atu()
	{
		for (unsigned i = 0; i < 10; i++) {
			if (controller.atu_up())
				return true;
			timer.msleep(10);
		}
		return true;
	}

	Main(Env & env) : env(env)
	{
		gpio.direction(Gpio::Session::OUT);
		gpio.write(false);
		timer.msleep(100);
		gpio.write(true);

		timer.msleep(100);
		phy_reset.construct(platform, Platform::Device::Type({"reset-pin,phy"}));

		controller.init();

		/*
		 * Force Gen1 when starting the link. In case the link is
		 * started in Gen2 mode, there is a possibility the devices on the
		 * bus will not be detected at all.
		 */
		controller.write<Pcie_controller::Link_capabilities::Max_link_speed>(1);

		core_reset.construct(platform, Platform::Device::Type({"reset-pin,core"}));

		if (!wait_for_link())
			throw Link_is_not_ready();

		controller.write<Pcie_controller::Link_capabilities::Max_link_speed>(2);
		controller.write<Pcie_controller::Link_width_speed_control::Direct_speed_change>(1);

		if (!wait_for_link())
			throw Link_is_not_ready();

		controller.configure_outbound_atu(0, Pcie_controller::CONFIG_TYPE_0_REQUEST,
		                                  0x1ff00000, 0x80000, 1<<24);
		if (!wait_for_atu())
			throw Atu_is_not_ready();

		log("Ready to probe!");
		log(Hex(cfg_space.read<Pcie_config_space::Id>()));
	}
};


void Component::construct(Env & env) {
	static Main main(env); }
