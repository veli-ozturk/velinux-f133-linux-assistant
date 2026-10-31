// SPDX-License-Identifier: GPL-2.0
/*
 * F133 10.26" panel (TLCMB1026S1B145-CN, 600x1600 MIPI-DSI 4 hat).
 * Melis'teki "he0801a068" adini korur; Tina'nin orijinal he0801a068.c dosyasinin yerine gecer.
 *
 * Guc sirasi Melis kernel'inden (0xa4200 civari) okundu:
 *   standart power/pin_cfg -> lcd_avdd(PB3)=1 -> lcd_power(PD19) 0->1 -> lcd_reset(PE9) 0, 10ms, 1, 20ms -> mipi_reg
 * Melis VCOM/AVDD PWM'lerine (fex [vcom]) kernel'de dokunmuyor, burada da surulmuyor.
 *
 * DTS: lcd_gpio_0 = reset (PE9), lcd_gpio_1 = lcd_power (PD19), lcd_gpio_2 = lcd_avdd (PB3).
 */
#include "he0801a068.h"

#define GPIO_RESET	0
#define GPIO_POWER	1
#define GPIO_AVDD	2

#define REGFLAG_DELAY		0x100
#define REGFLAG_END_OF_TABLE	0x101

struct LCM_setting_table {
	u32 cmd;
	u32 count;
	u8 para_list[64];
};

/* he0801a068_init_table.h, work\fex_to_linux_v1.py ile fex [mipi_reg]'den uretilir */
#include "he0801a068_init_table.h"

static void lcd_power_on(u32 sel);
static void lcd_power_off(u32 sel);
static void lcd_bl_open(u32 sel);
static void lcd_bl_close(u32 sel);
static void lcd_panel_init(u32 sel);
static void lcd_panel_exit(u32 sel);

static void lcd_cfg_panel_info(struct panel_extend_para *info)
{
	/* lcd_gamma_en = 0, lcd_cmap_en = 0: tablo gerekmiyor */
}

static s32 lcd_open_flow(u32 sel)
{
	LCD_OPEN_FUNC(sel, lcd_power_on, 10);
	LCD_OPEN_FUNC(sel, lcd_panel_init, 10);
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 50);
	LCD_OPEN_FUNC(sel, lcd_bl_open, 0);

	return 0;
}

static s32 lcd_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, lcd_bl_close, 0);
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
	LCD_CLOSE_FUNC(sel, lcd_panel_exit, 200);
	LCD_CLOSE_FUNC(sel, lcd_power_off, 500);

	return 0;
}

static void lcd_power_on(u32 sel)
{
	pr_info("he0801a068: power on\n");
	sunxi_lcd_power_enable(sel, 0);
	sunxi_lcd_delay_ms(10);
	sunxi_lcd_pin_cfg(sel, 1);
	sunxi_lcd_delay_ms(10);

	sunxi_lcd_gpio_set_value(sel, GPIO_AVDD, 1);
	sunxi_lcd_delay_ms(1);
	sunxi_lcd_gpio_set_value(sel, GPIO_POWER, 0);
	sunxi_lcd_delay_ms(1);
	sunxi_lcd_gpio_set_value(sel, GPIO_POWER, 1);
	sunxi_lcd_delay_ms(20);

	sunxi_lcd_gpio_set_value(sel, GPIO_RESET, 0);
	sunxi_lcd_delay_ms(10);
	sunxi_lcd_gpio_set_value(sel, GPIO_RESET, 1);
	sunxi_lcd_delay_ms(20);
}

static void lcd_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_delay_ms(20);
	sunxi_lcd_gpio_set_value(sel, GPIO_RESET, 0);
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_gpio_set_value(sel, GPIO_POWER, 0);
	sunxi_lcd_gpio_set_value(sel, GPIO_AVDD, 0);
	sunxi_lcd_power_disable(sel, 0);
}

static void lcd_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);
}

static void lcd_bl_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel);
	sunxi_lcd_pwm_disable(sel);
}

static void lcd_panel_init(u32 sel)
{
	u32 i;

	sunxi_lcd_dsi_clk_enable(sel);
	sunxi_lcd_delay_ms(10);

	for (i = 0; lcm_initialization_setting[i].cmd != REGFLAG_END_OF_TABLE; i++) {
		const struct LCM_setting_table *t = &lcm_initialization_setting[i];

		if (t->cmd == REGFLAG_DELAY)
			sunxi_lcd_delay_ms(t->count);
		else
			sunxi_lcd_dsi_dcs_write(sel, (u8)t->cmd, (u8 *)t->para_list, t->count);
	}
	pr_info("he0801a068: init dizisi gonderildi (%u satir)\n", i);
}

static void lcd_panel_exit(u32 sel)
{
	sunxi_lcd_dsi_dcs_write_0para(sel, 0x28);
	sunxi_lcd_delay_ms(20);
	sunxi_lcd_dsi_dcs_write_0para(sel, 0x10);
	sunxi_lcd_delay_ms(120);
}

/*sel: 0:lcd0; 1:lcd1*/
static s32 lcd_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

struct __lcd_panel he0801a068_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "he0801a068",
	.func = {
		.cfg_panel_info = lcd_cfg_panel_info,
		.cfg_open_flow = lcd_open_flow,
		.cfg_close_flow = lcd_close_flow,
		.lcd_user_defined_func = lcd_user_defined_func,
	},
};
