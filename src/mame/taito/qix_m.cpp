// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Zsolt Vasvari
// thanks-to: John Butler, Ed Mueller
/***************************************************************************

    Taito Qix hardware

***************************************************************************/

#include "emu.h"
#include "qix.h"


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void slither_state::machine_start()
{
	qix_state::machine_start();

	save_item(NAME(m_sn76489_ctrl));
}

void qixmcu_state::machine_start()
{
	qix_state::machine_start();

	// HACK: force port B latch to zero to avoid spurious coin counter pulses because the MCU program initializes DDRB before the latch
	m_mcu->set_state_int(m68705_device::M68705_LATCHB, 0);

	// reset the coin counter register
	m_coinctrl = 0x00;

	// set up save states
	save_item(NAME(m_68705_porta_out));
	save_item(NAME(m_coinctrl));
}

void zookeep_state::machine_start()
{
	if (m_mcu)
		qixmcu_state::machine_start();
	else
		qix_state::machine_start();

	// configure the banking
	m_vidbank->configure_entry(0, memregion("videocpu")->base() + 0xa000);
	m_vidbank->configure_entry(1, memregion("videocpu")->base() + 0x10000);
	m_vidbank->set_entry(0);
}

/*************************************
 *
 *  VSYNC change callback
 *
 *************************************/

void qix_state::qix_vsync_changed(int state)
{
	m_sndpia0->cb1_w(state);
}



/*************************************
 *
 *  Zoo Keeper bankswitching
 *
 *************************************/

void zookeep_state::bankswitch_w(uint8_t data)
{
	m_vidbank->set_entry(BIT(data, 2));
	// not necessary, but technically correct
	qix_palettebank_w(data);
}



/*************************************
 *
 *  Data CPU FIRQ generation/ack
 *
 *************************************/

void qix_state::qix_data_firq_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


void qix_state::qix_data_firq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


uint8_t qix_state::qix_data_firq_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	return space.unmap();
}


uint8_t qix_state::qix_data_firq_ack_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return space.unmap();
}



/*************************************
 *
 *  Video CPU FIRQ generation/ack
 *
 *************************************/

void qix_state::qix_video_firq_w(uint8_t data)
{
	m_videocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
}


void qix_state::qix_video_firq_ack_w(uint8_t data)
{
	m_videocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


uint8_t qix_state::qix_video_firq_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		m_videocpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	return space.unmap();
}


uint8_t qix_state::qix_video_firq_ack_r(address_space &space)
{
	if (!machine().side_effects_disabled())
		m_videocpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
	return space.unmap();
}



/*************************************
 *
 *  68705 Communication
 *
 *************************************/

uint8_t qixmcu_state::coin_r()
{
	logerror("qixmcu_state, coin_r = %02X\n", m_68705_porta_out);
	return m_68705_porta_out;
}


void qixmcu_state::coin_w(uint8_t data)
{
	logerror("qixmcu_state, coin_w = %02X\n", data);
	// this is a callback called by pia6821_device::write(), so I don't need to synchronize
	// the CPUs - they have already been synchronized by qix_pia_w()
	m_mcu->pa_w(data);
}


void qixmcu_state::coinctrl_w(uint8_t data)
{
	if (BIT(data, 2))
	{
		m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
		// temporarily boost the interleave to sync things up
		// note: I'm using 50 because 30 is not enough for space dungeon at game over
		machine().scheduler().perfect_quantum(attotime::from_usec(50));
	}
	else
	{
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}

	// this is a callback called by pia6821_device::write(), so I don't need to synchronize
	// the CPUs - they have already been synchronized by qix_pia_w()
	m_coinctrl = data;
	logerror("qixmcu_state, coinctrl_w = %02X\n", data);
}



/*************************************
 *
 *  68705 Port Inputs
 *
 *************************************/

uint8_t qixmcu_state::mcu_portb_r()
{
	return (m_coin->read() & 0x0f) | ((m_coin->read() & 0x80) >> 3);
}


uint8_t qixmcu_state::mcu_portc_r()
{
	return (m_coinctrl & 0x08) | ((m_coin->read() & 0x70) >> 4);
}



/*************************************
 *
 *  68705 Port Outputs
 *
 *************************************/

void qixmcu_state::mcu_porta_w(uint8_t data)
{
	logerror("68705:portA_w = %02X\n", data);
	m_68705_porta_out = data;
}


void qixmcu_state::mcu_portb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	data &= mem_mask;
	machine().bookkeeping().coin_lockout_w(0, (~data >> 6) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 7) & 1);
}



/*************************************
 *
 *  Data CPU PIA 0 synchronization
 *
 *************************************/

TIMER_CALLBACK_MEMBER(qix_state::pia_w_callback)
{
	m_pia0->write(param >> 8, param & 0xff);
}


void qix_state::qix_pia_w(offs_t offset, uint8_t data)
{
	// make all the CPUs synchronize, and only AFTER that write the command to the PIA
	// otherwise the 68705 will miss commands
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(qix_state::pia_w_callback), this), data | (offset << 8));
}



/*************************************
 *
 *  Coin I/O for games without coin CPU
 *
 *************************************/

void qix_state::qix_coinctl_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, (~data >> 2) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 1) & 1);
}

void slither_state::slither_coinctl_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, (~data >> 6) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 5) & 1);
}



/*************************************
 *
 *  Slither SN76489 I/O
 *
 *************************************/

void slither_state::sn76489_0_ctrl_w(int state)
{
	// write to the sound chip
	if (!state && m_sn76489_ctrl[0])
		m_sn[0]->write(m_pia1->b_output());

	m_sn76489_ctrl[0] = bool(state);
}


void slither_state::sn76489_1_ctrl_w(int state)
{
	// write to the sound chip
	if (!state && m_sn76489_ctrl[1])
		m_sn[1]->write(m_pia2->b_output());

	m_sn76489_ctrl[1] = bool(state);
}



/*************************************
 *
 *  Slither trackball I/O
 *
 *************************************/

uint8_t slither_state::trak_lr_r()
{
	return m_trak[m_flip ? 3 : 1]->read();
}


uint8_t slither_state::trak_ud_r()
{
	return m_trak[m_flip ? 2 : 0]->read();
}