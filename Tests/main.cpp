#include "gtest/gtest.h"

#include <gtk/gtk.h>

static int OnCommandLine(GtkApplication *app, GApplicationCommandLine *cmdline)
{
    int argc;
    char **argv = g_application_command_line_get_arguments(cmdline, &argc);
    testing::InitGoogleTest(&argc, argv);
    g_strfreev(argv);

    return RUN_ALL_TESTS();
}

int main(int argc, char **argv)
{
    auto app = gtk_application_new("com.patrickfournier.gorfector-tests", G_APPLICATION_HANDLES_COMMAND_LINE);
    g_signal_connect(app, "command-line", G_CALLBACK(OnCommandLine), nullptr);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
