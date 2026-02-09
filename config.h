/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx			 =  1;        /* border pixel of windows */
static const unsigned int snap           = 32;        /* snap pixel */
static const unsigned int systraypinning =  0;        /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft  =  0;        /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing =  2;        /* systray spacing */
static const int systraypinningfailfirst =  1;        /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray             =  1;        /* 0 means no systray */
static const int showbar                 =  1;        /* 0 means no bar */
static const int topbar                  =  1;        /* 0 means bottom bar */
/*  Display modes of the tab bar: never shown, always shown, shown only in  */
/*  monocle mode in the presence of several windows.                        */
/*  Modes after showtab_nmodes are disabled.                                */
enum showtab_modes { showtab_never, showtab_auto, showtab_nmodes, showtab_always};
static const int showtab			           = showtab_auto;        /* Default tab bar show mode */
static const int toptab				           = 1;                   /* 0 means bottom tab bar */

static const char default_font[]    = "FontAwesomeNerdFont-12";
static const char *fonts[]          = { default_font, "monospace:size=12" };
#define					  dmenufont           default_font

static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#9899a0";
static const char col_gray3[]       = "#586e75";
static const char col_gray4[]       = "#E5E9F0";
static const char col_cyan[]        = "#002b36";
#define						col_dmenu_sf        col_gray4
#define						col_dmenu_sb			  col_gray3
#define						col_dmenu_nf			  col_gray2
#define						col_dmenu_nb			  col_cyan 
static const char *colors[][3]      = {
	/*                   fg         bg          border   */
	[SchemeNorm    ] = { col_gray3, col_cyan  , col_gray1  },
	[SchemeSel     ] = { col_gray4, col_gray3 , col_gray1  },
	[SchemeStatus  ] = { col_gray4, col_gray1 , "#000000"  }, // Statusbar right {text,background,not used but cannot be empty}
  [SchemeTagsSel ] = { col_gray4, col_gray3 , "#000000"  }, // Tagbar left selected {text,background,not used but cannot be empty}
  [SchemeTagsNorm] = { col_gray4, col_cyan  , "#000000"  }, // Tagbar left unselected {text,background,not used but cannot be empty}
  [SchemeTabsSel ] = { col_gray4, col_gray3 , "#000000"  }, // Tagbar left selected {text,background,not used but cannot be empty}
  [SchemeTabsNorm] = { col_gray4, col_cyan  , "#000000"  }, // Tagbar left unselected {text,background,not used but cannot be empty}
  [SchemeInfoSel ] = { col_gray4, col_gray1 , "#000000"  }, // infobar middle  selected {text,background,not used but cannot be empty}
  [SchemeInfoNorm] = { col_gray4, col_gray1 , "#000000"  }, // infobar middle  unselected {text,background,not used but cannot be empty}
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

// NOTE: See https://dwm.suckless.org/customisation/rules/
static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ NULL,       NULL,       NULL,       0,            False,       -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 120;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask // Use Windows key
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
#define dmenu_common_flags "-m", dmenumon, "-fn", dmenufont, "-l", "15", "-i", "-nb", col_dmenu_nb ,"-nf", col_dmenu_nf, "-sb", col_dmenu_sb ,"-sf", col_dmenu_sf
static const char *dmenucmd[]				      = { "dmenu_run",                 dmenu_common_flags, "-p", "Bin:", NULL };
static const char *dmenudesktopcmd[]      = { "dmenu-desktop.sh",          dmenu_common_flags, "-p", "Software:", NULL };
static const char *dmenufilecmd[]         = { "dmenu-find-file.sh",        dmenu_common_flags, "-p", "File:", NULL };
static const char *dmenuhiddenfilecmd[]   = { "dmenu-find-hidden-file.sh", dmenu_common_flags, "-p", "File:", NULL };

static const char *termcmd[]              = { "sensible-terminal.sh", NULL };
static const char *browsercmd[]           = { "flatpak", "run", "com.brave.Browser", NULL };

static const char *mailcmd[]              = { "sensible-terminal.sh", "-e", "neomutt_wrapper.sh", NULL };
static const char *whatsappcmd[]          = { "flatpak", "run", "com.rtosta.zapzap", NULL };
static const char *telegramcmd[]          = { "flatpak", "run", "org.telegram.desktop", NULL };
static const char *fileexplorercmd[]      = { "thunar", NULL };
static const char *lockcmd[]              = { "lock.sh", NULL };
static const char *toggleaudiocmd[]       = { "toggle-sink.sh", NULL };
static const char *popnotificationcmd[]   = { "dunstctl", "history-pop", NULL };
static const char *closenotificationcmd[] = { "dunstctl", "close-all", NULL };

#define PACTL(cmd, arg) SHCMD("pactl " cmd " @DEFAULT_SINK@ " arg "; pkill -RTMIN+3 $STATUSBAR")
#define            upvolumecmd              PACTL("set-sink-volume", "+5%")
#define            downvolumecmd            PACTL("set-sink-volume", "-5%")
#define            mutevolumecmd            PACTL("set-sink-mute", "toggle")
static const char *mutemiccmd[]           = { "pactl", "set-sink-mute",   "@DEFAULT_SOURCE@", "toggle", NULL };

static const char *brightercmd[]          = { "brightnessctl", "set", "5%+", NULL };
static const char *dimmercmd[]            = { "brightnessctl", "set", "5%-", NULL };

#define screenshot_path							    "$HOME/Pictures/Screenshots/"
#define screenshot_file							    screenshot_path"$(date +%Y-%m-%dT%H-%M-%S).png"
#define create_screenshot_path          "mkdir -p "screenshot_path" && "
#define screenshotcmd								    SHCMD(create_screenshot_path"maim " screenshot_file)
#define screenshotselectcmd					    SHCMD(create_screenshot_path"maim --select " screenshot_file)
#define screenshotclipboardcmd          SHCMD("maim | xclip -selection clipboard -t image/png")
#define screenshotselectsclipboardcmd   SHCMD("maim --select | xclip -selection clipboard -t image/png")

// Cal
static const char* calcmd[]           = { "cal-notify.sh", NULL };

// Jira
#define jiracmd                         SHCMD("sensible-terminal.sh -e $HOME/.config/jira/jira.sh")
#define jirasearchcmd                   SHCMD("sensible-terminal.sh -e $HOME/.config/jira/jira.sh --search")
#define jiraallcmd                      SHCMD("sensible-terminal.sh -e $HOME/.config/jira/jira.sh --all")

static const Key keys[] = {
	/* modifier                     key                       function        argument */
	{ MODKEY,                       XK_d,                     spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_g,                     spawn,          {.v = dmenudesktopcmd } },
	{ MODKEY,                       XK_o,                     spawn,          {.v = dmenufilecmd } },
	{ MODKEY|ShiftMask,             XK_o,                     spawn,          {.v = dmenuhiddenfilecmd } },
	{ MODKEY,                       XK_Return,                spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_i,                     spawn,          {.v = browsercmd } },
	{ MODKEY,                       XK_m,                     spawn,          {.v = mailcmd } },
	{ MODKEY,                       XK_t,                     spawn,          {.v = whatsappcmd } },
	{ MODKEY|ShiftMask,             XK_t,                     spawn,          {.v = telegramcmd } },
	{ MODKEY,                       XK_y,                     spawn,          {.v = fileexplorercmd } },
	{ MODKEY,                       XK_a,                     spawn,          {.v = toggleaudiocmd } },
	{ MODKEY,                       XK_n,                     spawn,          {.v = popnotificationcmd } },
	{ MODKEY|ShiftMask,             XK_n,                     spawn,          {.v = closenotificationcmd } },
	{ MODKEY,                       XK_Escape,                spawn,          {.v = lockcmd } },
  { 0,                            XF86XK_AudioMute,         spawn,          mutevolumecmd  },
  { 0,                            XF86XK_AudioMicMute,      spawn,          {.v = mutemiccmd } },
  { 0,                            XF86XK_AudioLowerVolume,  spawn,          downvolumecmd  },
	{ 0,                            XF86XK_AudioRaiseVolume,  spawn,          upvolumecmd  },
  { 0,                            XF86XK_MonBrightnessDown, spawn,          {.v = dimmercmd } },
  { 0,                            XF86XK_MonBrightnessUp,   spawn,          {.v = brightercmd } },
	{ 0,                            XK_Print,                 spawn,          screenshotcmd },
	{ ShiftMask,                    XK_Print,                 spawn,          screenshotselectcmd },
	{ ControlMask,                  XK_Print,                 spawn,          screenshotclipboardcmd },
	{ ControlMask|ShiftMask,        XK_Print,                 spawn,          screenshotselectsclipboardcmd },
	{ MODKEY,                       XK_c,                     spawn,          {.v = calcmd } },
	{ MODKEY,                       XK_p,                     spawn,          jiracmd },
	{ MODKEY|ShiftMask,             XK_p,                     spawn,          jirasearchcmd },
	{ MODKEY|ControlMask,           XK_p,                     spawn,          jiraallcmd },
	{ MODKEY,                       XK_b,                     togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_w,											tabmode,        {-1} },
	{ MODKEY,                       XK_j,                     focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,                     focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_i,                     incnmaster,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_d,                     incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,                     setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,                     setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_Return,                zoom,           {0} },
	{ MODKEY,                       XK_Tab,                   view,           {0} },
	{ MODKEY|ShiftMask,             XK_q,                     killclient,     {0} },
	{ MODKEY,                       XK_e,                     setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,                     setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_w,                     setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,                 setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,                 togglefloating, {0} },
	{ MODKEY,                       XK_0,                     view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                     tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,                 focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period,                focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,                 tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                tagmon,         {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_apostrophe,            swapmon,        {0} },
	TAGKEYS(                        XK_1,                                      0)
	TAGKEYS(                        XK_2,                                      1)
	TAGKEYS(                        XK_3,                                      2)
	TAGKEYS(                        XK_4,                                      3)
	TAGKEYS(                        XK_5,                                      4)
	TAGKEYS(                        XK_6,                                      5)
	TAGKEYS(                        XK_7,                                      6)
	TAGKEYS(                        XK_8,                                      7)
	TAGKEYS(                        XK_9,                                      8)
	{ MODKEY|ShiftMask,             XK_BackSpace,             quit,           {0} },  // Terminates dwm
	{ MODKEY|ShiftMask,             XK_r,                     quit,           {1} },  // Restarts dwm
	{ MODKEY,                       XK_s,                     togglesticky,   {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigstatusbar,   {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,   {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,   {.i = 3} },
	{ ClkStatusText,        0,              Button4,        sigstatusbar,   {.i = 4} },
	{ ClkStatusText,        0,              Button5,        sigstatusbar,   {.i = 5} },
	{ ClkStatusText,        0,              6,              sigstatusbar,   {.i = 6} },
	{ ClkStatusText,        0,              7,              sigstatusbar,   {.i = 7} },
	{ ClkStatusText,        0,              8,              sigstatusbar,   {.i = 8} },
	{ ClkStatusText,        0,              9,              sigstatusbar,   {.i = 9} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkTabBar,            0,              Button1,        focuswin,       {0} },
};

