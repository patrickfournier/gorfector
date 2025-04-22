#pragma once

#include <libintl.h>
#include <locale.h>

#define _(string) gettext(string)
#define gettext_noop(string) string
#define N_(string) gettext_noop(string)
