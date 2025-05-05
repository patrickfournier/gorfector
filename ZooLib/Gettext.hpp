#pragma once

#include <clocale>
#include <libintl.h>

#define _(string) gettext(string)
#define gettext_noop(string) string
#define N_(string) gettext_noop(string)
