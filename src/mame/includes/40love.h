// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#ifndef MAME_INCLUDES_40LOVE_H
#define MAME_INCLUDES_40LOVE_H

#pragma once

#include "machine/taito68705interface.h"
#include "machine/gen_latch.h"
#include "sound/msm5232.h"
#include "sound/ay8910.h"
#include "sound/ta7630.h"
#include "emupal.h"
#include "tilemap.h"

class fortyl_state : public driver_device
{
public:
	fortyl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_maincpu(*this, "maincpu")
		, m_bmcu(*this, "bmcu")
		, m_msm(*this, "msm")
		, m_ay(*this,"aysnd")
		, m_ta7630(*this,"ta7630")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch2(*this, "soundlatch2")
		, m_videoram(*this, "videoram")
		, m_video_ctrl(*this, "video_ctrl")
		, m_spriteram(*this, "spriteram")
		, m_colorram(*this, "colorram")
		, m_spriteram2(*this, "spriteram2")
		, m_mcu_ram(*this, "mcu_ram")
	{ }

	virtual void driver_init() override;

	uint32_t screen_update_fortyl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void redraw_pixels();
	void fortyl_set_scroll_x( int offset );
	void fortyl_plot_pix( int offset );
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_pixram( bitmap_ind16 &bitmap, const rectangle &cliprect );

	void common(machine_config &config);
	void _40love(machine_config &config);
	void undoukai(machine_config &config);

	void _40love_map(address_map &map);
	void sound_map(address_map &map);
	void undoukai_map(address_map &map);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void bank_select_w(uint8_t data);
	uint8_t snd_flag_r();
	uint8_t fortyl_mcu_status_r();
	void fortyl_pixram_sel_w(uint8_t data);
	uint8_t fortyl_pixram_r(offs_t offset);
	void fortyl_pixram_w(offs_t offset, uint8_t data);
	void fortyl_bg_videoram_w(offs_t offset, uint8_t data);
	uint8_t fortyl_bg_videoram_r(offs_t offset);
	void fortyl_bg_colorram_w(offs_t offset, uint8_t data);
	uint8_t fortyl_bg_colorram_r(offs_t offset);
	void sound_control_0_w(uint8_t data);
	void sound_control_1_w(uint8_t data);
	void sound_control_2_w(uint8_t data);
	void sound_control_3_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<msm5232_device> m_msm;
	required_device<ym2149_device> m_ay;
	required_device<ta7630_device> m_ta7630;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch2;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_video_ctrl;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram2;
	optional_shared_ptr<uint8_t> m_mcu_ram;

	/* video-related */
	std::unique_ptr<bitmap_ind16>    m_tmp_bitmap1;
	std::unique_ptr<bitmap_ind16>    m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	uint8_t       m_flipscreen;
	uint8_t       m_pix_redraw;
	uint8_t       m_xoffset;
	std::unique_ptr<uint8_t[]>       m_pixram1;
	std::unique_ptr<uint8_t[]>       m_pixram2;
	bitmap_ind16    *m_pixel_bitmap1;
	bitmap_ind16    *m_pixel_bitmap2;
	int         m_pixram_sel;
	bool        m_color_bank;
	bool        m_screen_disable;

	/* misc */
	int         m_pix_color[4];
	int         m_vol_ctrl[16];
	uint8_t       m_snd_ctrl0;
	uint8_t       m_snd_ctrl1;
	uint8_t       m_snd_ctrl2;
	uint8_t       m_snd_ctrl3;
};

#endif // MAME_INCLUDES_40LOVE_H