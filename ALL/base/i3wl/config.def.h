/* Taken from https://github.com/djpohly/dwl/issues/466 */
#define COLOR(hex)    { ((hex >> 24) & 0xFF) / 255.0f, \
                        ((hex >> 16) & 0xFF) / 255.0f, \
                        ((hex >> 8) & 0xFF) / 255.0f, \
                        (hex & 0xFF) / 255.0f }
#include "glyphs.h"

/* appearance */
static const int sloppyfocus               = 1;  /* focus follows mouse */
static const int bypass_surface_visibility = 0;  /* 1 means idle inhibitors will disable idle tracking even if it's surface isn't visible  */
static const int smartgaps                 = 0;  /* 1 means no outer gap when there is only one window */
static int gaps                            = 1;  /* 1 means gaps between windows are added */
static const unsigned int gappx            = 10; /* gap pixel between windows */
static const unsigned int borderpx         = 1;  /* border pixel of windows */
static const unsigned int systrayspacing   = 8;  /* systray spacing */
static const int showsystray               = 1;  /* 0 means no systray */
static const int showbar                   = 1;  /* 0 means no bar */
static const int topbar                    = 1;  /* 0 means bottom bar */
static const char *fonts[]                 = {"JetBrainsMono Nerd Font Mono:size=11:weight=bold"};
static const float rootcolor[]             = COLOR(0x000000ff);
/* This conforms to the xdg-protocol. Set the alpha to zero to restore the old behavior */
static const float fullscreen_bg[]         = {0.0f, 0.0f, 0.0f, 1.0f}; /* You can also use glsl colors */
static uint32_t colors[][3]                = {
	/*               fg          bg          border    */
	[SchemeNorm] = { 0xbbbbbbff, 0x222222ff, 0x444444ff },
	[SchemeSel]  = { 0xeeeeeeff, 0x005577ff, 0x005577ff },
	[SchemeUrg]  = { 0,          0,          0x770000ff },
};

static const float resize_factor           = 0.0002f; /* Resize multiplier for mouse resizing, depends on mouse sensivity. */
static const uint32_t resize_interval_ms   = 16; /* Resize interval depends on framerate and screen refresh rate. */

enum Direction { DIR_LEFT, DIR_RIGHT, DIR_UP, DIR_DOWN };

/* bar layout — floating pills, sized/colored to match the reference waybar rice */
static const unsigned int barheight   = 34; /* logical px, bar content height */
static const unsigned int barmarginx  = 12; /* logical px, left/right screen inset */
static const unsigned int barmarginy  = 8;  /* logical px, top screen inset */
static const unsigned int pillgap     = 8;  /* logical px, gap between top-level pills */
static const unsigned int pillradius  = 10; /* logical px, pill corner radius */
static const unsigned int pillpadx    = 12; /* logical px, horizontal text padding inside a pill */

/* bar colors — 0xRRGGBBAA, same encoding as COLOR()/colors[] above */
static const uint32_t pillbg        = 0x1c1c1cd1; /* rgba(28,28,28,.82) */
static const uint32_t pillborder    = 0x303030ff; /* 1px ring, drawn as an outer fill + inset inner fill */
static const uint32_t pillfg        = 0xffffffff;
static const uint32_t clockbg       = 0x252525d9; /* rgba(37,37,37,.85) */
static const uint32_t wsinactivefg  = 0x5a5a5aff;
static const uint32_t wsfocusedbg   = 0xffffff12; /* rgba(255,255,255,.07) */
static const uint32_t wsurgentfg    = 0xb06a6aff;
static const uint32_t powerhoverfg  = 0xb06a6aff;

/* tagging */
static char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" };

/* logging */
static int log_level = WLR_ERROR;

static const Rule rules[] = {
	/* app_id             title       tags mask     isfloating   monitor */
	{ "Gimp_EXAMPLE",     NULL,       0,            1,           -1 }, /* Start on currently visible tags floating, not tiled */
	{ "firefox_EXAMPLE",  NULL,       1 << 8,       0,           -1 }, /* Start on ONLY tag "9" */
    /* default/example rule: can be changed but cannot be eliminated; at least one rule must exist */
};

/* layout(s) */
static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "|w|",      btrtile }, /* i3-style focus-driven binary-split tiling (default) */
	{ "[]=",      tile },
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* monitors */
/* (x=-1, y=-1) is reserved as an "autoconfigure" monitor position indicator
 * WARNING: negative values other than (-1, -1) cause problems with Xwayland clients due to
 * https://gitlab.freedesktop.org/xorg/xserver/-/issues/899 */
static const MonitorRule monrules[] = {
   /* name        mfact  nmaster scale layout       rotate/reflect                x    y
    * example of a HiDPI laptop monitor:
    { "eDP-1",    0.5f,  1,      2,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 }, */
	{ NULL,       0.55f, 1,      1,    &layouts[0], WL_OUTPUT_TRANSFORM_NORMAL,   -1,  -1 },
	/* default monitor rule: can be changed but cannot be eliminated; at least one monitor rule must exist */
};

/* keyboard */
static const struct xkb_rule_names xkb_rules = {
	/* can specify fields: rules, model, layout, variant, options */
	/* example:
	.options = "ctrl:nocaps",
	*/
	.options = NULL,
};

static const int repeat_rate = 25;
static const int repeat_delay = 600;

/* Trackpad */
static const int tap_to_click = 1;
static const int tap_and_drag = 1;
static const int drag_lock = 1;
static const int natural_scrolling = 0;
static const int disable_while_typing = 1;
static const int left_handed = 0;
static const int middle_button_emulation = 0;
/* You can choose between:
LIBINPUT_CONFIG_SCROLL_NO_SCROLL
LIBINPUT_CONFIG_SCROLL_2FG
LIBINPUT_CONFIG_SCROLL_EDGE
LIBINPUT_CONFIG_SCROLL_ON_BUTTON_DOWN
*/
static const enum libinput_config_scroll_method scroll_method = LIBINPUT_CONFIG_SCROLL_2FG;

/* You can choose between:
LIBINPUT_CONFIG_CLICK_METHOD_NONE
LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS
LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
*/
static const enum libinput_config_click_method click_method = LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;

/* You can choose between:
LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED
LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE
*/
static const uint32_t send_events_mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;

/* You can choose between:
LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT
LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
*/
static const enum libinput_config_accel_profile accel_profile = LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE;
static const double accel_speed = 0.0;

/* You can choose between:
LIBINPUT_CONFIG_TAP_MAP_LRM -- 1/2/3 finger tap maps to left/right/middle
LIBINPUT_CONFIG_TAP_MAP_LMR -- 1/2/3 finger tap maps to left/middle/right
*/
static const enum libinput_config_tap_button_map button_map = LIBINPUT_CONFIG_TAP_MAP_LRM;

/* i3 uses the Super/Windows key as $mod by default */
#define MODKEY WLR_MODIFIER_LOGO

#define TAGKEYS(KEY,SKEY,TAG) \
	{ MODKEY,                    KEY,            view,            {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL,  KEY,            toggleview,      {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_SHIFT, SKEY,           tag,             {.ui = 1 << TAG} }, \
	{ MODKEY|WLR_MODIFIER_CTRL|WLR_MODIFIER_SHIFT,SKEY,toggletag, {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *termcmd[]      = { "foot", NULL };
static const char *filemgrcmd[]   = { "nemo", NULL };
static const char *browsercmd[]   = { "firefox", NULL };
static const char *lockcmd[]      = { "/home/apiwo/.config/sway/scripts/lock.sh", NULL };
static const char *dmenucmd[]     = { "bemenu", "-i", NULL }; /* used for the tray right-click menu; wmenu isn't installed on this system */
static const char *powermenucmd[] = { "wlogout", "-b", "3", "-T", "300", "-B", "300", NULL };
static const char *statcpucmd[]   = { "foot", "-e", "btop", NULL };
static const char *statvolcmd[]   = { "wpctl", "set-mute", "@DEFAULT_AUDIO_SINK@", "toggle", NULL };
/* fuzzel isn't installed on this system; bemenu-run is the launcher actually
 * tuned in the reference rice (same colors as the sway/niri config used) */
static const char *menucmd[] = { "bemenu-run", "-i", "-p", "",
	"--fn", "JetBrainsMono Nerd Font 11",
	"--tb", "#1c1c1c", "--tf", "#ededed",
	"--fb", "#1c1c1c", "--ff", "#cacaca",
	"--nb", "#1c1c1c", "--nf", "#8a8a8a",
	"--ab", "#1c1c1c", "--af", "#cacaca",
	"--hb", "#303030", "--hf", "#ffffff",
	"--sb", "#303030", "--sf", "#ffffff",
	"--scb", "#1c1c1c", "--scf", "#dcdcdc",
	"--border", "2", "--bdr", "#4a4a4a", "--line-height", "26", NULL };
static const char *screenshotcmd[] = { "/bin/sh", "-c",
	"grim \"$HOME/Pictures/Screenshots/Screenshot from $(date '+%Y-%m-%d %H-%M-%S').png\"", NULL };
static const char *screenshotregioncmd[] = { "/bin/sh", "-c",
	"grim -g \"$(slurp)\" \"$HOME/Pictures/Screenshots/Screenshot from $(date '+%Y-%m-%d %H-%M-%S').png\"", NULL };

static const Key keys[] = {
	/* Note that Shift changes certain key codes: 2 -> at, etc. */
	/* modifier                  key                  function          argument */
	/* -- launchers (matching apiwo's niri config) -- */
	{ MODKEY,                    XKB_KEY_Return,      spawn,            {.v = termcmd} },
	{ MODKEY,                    XKB_KEY_d,           spawn,            {.v = menucmd} },
	{ MODKEY,                    XKB_KEY_n,           spawn,            {.v = filemgrcmd} },
	{ MODKEY,                    XKB_KEY_e,           spawn,            {.v = browsercmd} },
	{ MODKEY,                    XKB_KEY_l,           spawn,            {.v = lockcmd} }, /* l is reserved for lock, not focus (matches niri config) */
	{ MODKEY,                    XKB_KEY_b,           togglebar,        {0} },

	/* -- focus movement: arrows always spatial; j/k = down/up (apiwo's actual
	 * convention, not i3 stock's unusual j=left/k=down/l=up) -- */
	{ MODKEY,                    XKB_KEY_Left,        focusdir,         {.ui = 0} },
	{ MODKEY,                    XKB_KEY_Down,        focusdir,         {.ui = 3} },
	{ MODKEY,                    XKB_KEY_Up,          focusdir,         {.ui = 2} },
	{ MODKEY,                    XKB_KEY_Right,       focusdir,         {.ui = 1} },
	{ MODKEY,                    XKB_KEY_j,           focusdir,         {.ui = 3} },
	{ MODKEY,                    XKB_KEY_k,           focusdir,         {.ui = 2} },
	{ MODKEY,                    XKB_KEY_semicolon,   focusdir,         {.ui = 1} },

	/* -- move window: Shift+arrows and Shift+h/j/k/l (h/l free for this since
	 * plain h/l are reserved for lock/other, not focus) -- */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Left,        swapclients,      {.i = DIR_LEFT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Down,        swapclients,      {.i = DIR_DOWN} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Up,          swapclients,      {.i = DIR_UP} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Right,       swapclients,      {.i = DIR_RIGHT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_H,           swapclients,      {.i = DIR_LEFT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_J,           swapclients,      {.i = DIR_DOWN} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_K,           swapclients,      {.i = DIR_UP} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_L,           swapclients,      {.i = DIR_RIGHT} },

	/* live resize (btrtile split ratio) — Ctrl+arrows, since i3's modal resize mode has no dwl equivalent */
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Right,       setratio_h,       {.f = +0.025f} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Left,        setratio_h,       {.f = -0.025f} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Up,          setratio_v,       {.f = -0.025f} },
	{ MODKEY|WLR_MODIFIER_CTRL,  XKB_KEY_Down,        setratio_v,       {.f = +0.025f} },

	{ MODKEY,                    XKB_KEY_i,           incnmaster,       {.i = +1} },
	{ MODKEY,                    XKB_KEY_u,           incnmaster,       {.i = -1} },
	{ MODKEY,                    XKB_KEY_Tab,         view,             {0} },

	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_Q,           killclient,       {0} }, /* $mod+Shift+q: kill focused window */
	{ MODKEY,                    XKB_KEY_f,           togglefullscreen, {0} }, /* $mod+f: true fullscreen */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_F,           setlayout,        {.v = &layouts[3]} }, /* $mod+shift+f: monocle ("maximize", stays tiled) */
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_space,       togglefloating,   {0} }, /* $mod+Shift+space: floating toggle */
	{ MODKEY,                    XKB_KEY_space,       setlayout,        {0} }, /* cycle layout */
	{ MODKEY,                    XKB_KEY_t,           setlayout,        {.v = &layouts[0]} }, /* back to btrtile */
	{ MODKEY,                    XKB_KEY_g,           togglegaps,       {0} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_E,           spawn,            {.v = powermenucmd} }, /* $mod+Shift+e: power menu (matches niri config) */
	{ MODKEY,                    XKB_KEY_Escape,      spawn,            {.v = powermenucmd} }, /* $mod+Escape: power menu (matches niri config) */

	{ MODKEY,                    XKB_KEY_comma,       focusmon,         {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY,                    XKB_KEY_period,      focusmon,         {.i = WLR_DIRECTION_RIGHT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_less,        tagmon,           {.i = WLR_DIRECTION_LEFT} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_greater,     tagmon,           {.i = WLR_DIRECTION_RIGHT} },

	/* -- screenshots (matches niri config) -- */
	{ 0,                         XKB_KEY_Print,       spawn,            {.v = screenshotcmd} },
	{ MODKEY,                    XKB_KEY_Print,       spawn,            {.v = screenshotregioncmd} },
	{ MODKEY|WLR_MODIFIER_SHIFT, XKB_KEY_S,           spawn,            {.v = screenshotcmd} },

	/* -- media / brightness (matches niri config) -- */
	{ 0, XKB_KEY_XF86AudioRaiseVolume,  spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+") },
	{ 0, XKB_KEY_XF86AudioLowerVolume,  spawn, SHCMD("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-") },
	{ 0, XKB_KEY_XF86AudioMute,         spawn, SHCMD("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle") },
	{ 0, XKB_KEY_XF86AudioMicMute,      spawn, SHCMD("wpctl set-mute @DEFAULT_AUDIO_SOURCE@ toggle") },
	{ 0, XKB_KEY_XF86MonBrightnessUp,   spawn, SHCMD("brightnessctl set 5%+") },
	{ 0, XKB_KEY_XF86MonBrightnessDown, spawn, SHCMD("brightnessctl set 5%-") },

	TAGKEYS(          XKB_KEY_1, XKB_KEY_exclam,                        0),
	TAGKEYS(          XKB_KEY_2, XKB_KEY_at,                            1),
	TAGKEYS(          XKB_KEY_3, XKB_KEY_numbersign,                    2),
	TAGKEYS(          XKB_KEY_4, XKB_KEY_dollar,                        3),
	TAGKEYS(          XKB_KEY_5, XKB_KEY_percent,                       4),
	TAGKEYS(          XKB_KEY_6, XKB_KEY_asciicircum,                   5),
	TAGKEYS(          XKB_KEY_7, XKB_KEY_ampersand,                     6),
	TAGKEYS(          XKB_KEY_8, XKB_KEY_asterisk,                      7),
	TAGKEYS(          XKB_KEY_9, XKB_KEY_parenleft,                     8),
	TAGKEYS(          XKB_KEY_0, XKB_KEY_parenright,                    9), /* workspace 10, matches niri config */

	/* Ctrl-Alt-Backspace/Delete used to be handled by X server */
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_Terminate_Server, quit, {0} },
	{ WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_Delete, quit, {0} }, /* matches niri's Ctrl+Alt+Delete */
	/* Ctrl-Alt-Fx is used to switch to another VT, if you don't know what a VT is
	 * do not remove them.
	 */
#define CHVT(n) { WLR_MODIFIER_CTRL|WLR_MODIFIER_ALT,XKB_KEY_XF86Switch_VT_##n, chvt, {.ui = (n)} }
	CHVT(1), CHVT(2), CHVT(3), CHVT(4), CHVT(5), CHVT(6),
	CHVT(7), CHVT(8), CHVT(9), CHVT(10), CHVT(11), CHVT(12),
};

static const Button buttons[] = {
	{ ClkGentoo,   0,      BTN_LEFT,   spawn,          {.v = menucmd} },
	{ ClkPower,    0,      BTN_LEFT,   spawn,          {.v = powermenucmd} },
	{ ClkStat,     0,      BTN_LEFT,   statclick,      {0} },
	{ ClkClient,   MODKEY, BTN_LEFT,   moveresize,     {.ui = CurMove} },
	{ ClkClient,   MODKEY, BTN_MIDDLE, togglefloating, {0} },
	{ ClkClient,   MODKEY, BTN_RIGHT,  moveresize,     {.ui = CurResize} },
	{ ClkTagBar,   0,      BTN_LEFT,   view,           {0} },
	{ ClkTagBar,   0,      BTN_RIGHT,  toggleview,     {0} },
	{ ClkTagBar,   MODKEY, BTN_LEFT,   tag,            {0} },
	{ ClkTagBar,   MODKEY, BTN_RIGHT,  toggletag,      {0} },
	{ ClkTray,     0,      BTN_LEFT,   trayactivate,   {0} },
	{ ClkTray,     0,      BTN_RIGHT,  traymenu,       {0} },
};
